import torch
import torch.nn as nn
import torch.nn.functional as F
from loguru import logger

class WindowsInt4Linear(nn.Module):
    """A custom Linear layer that stores 4-bit weights as packed uint8.
    Designed for Windows compatibility and torch.compile optimization.
    """
    def __init__(self, in_features, out_features, bias=None, group_size=128, device=None, dtype=None):
        super().__init__()
        self.in_features = in_features
        self.out_features = out_features
        self.group_size = group_size
        
        # Packed weights: (out_features, in_features // 2)
        self.register_buffer(
            "packed_weight", 
            torch.zeros((out_features, in_features // 2), dtype=torch.uint8, device=device)
        )
        
        # Scales and zero points: (out_features, in_features // group_size)
        num_groups = in_features // group_size
        self.register_buffer(
            "scale", 
            torch.ones((out_features, num_groups), dtype=dtype, device=device)
        )
        self.register_buffer(
            "zero_point", 
            torch.zeros((out_features, num_groups), dtype=dtype, device=device)
        )
        
        if bias is not None:
            self.bias = nn.Parameter(bias.to(device=device, dtype=dtype))
        else:
            self.register_parameter("bias", None)

    @classmethod
    def from_linear(cls, linear_module, group_size=128):
        """Create a WindowsInt4Linear from a standard nn.Linear."""
        device = linear_module.weight.device
        dtype = linear_module.weight.dtype
        
        # Quantize original weight
        w = linear_module.weight.data
        out_features, in_features = w.shape
        
        # Reshape for groupwise quantization
        w_reshaped = w.view(out_features, -1, group_size)
        
        # Compute scales and zero points (asymmetric)
        mn = w_reshaped.min(dim=-1, keepdim=True).values
        mx = w_reshaped.max(dim=-1, keepdim=True).values
        scale = (mx - mn) / 15.0
        scale = scale.clamp(min=1e-6)
        zp = mn
        
        # Quantize and clamp to 0-15
        wq = ((w_reshaped - mn) / scale).round().clamp(0, 15).to(torch.uint8)
        
        # Pack (out_features, in_features) -> (out_features, in_features // 2)
        wq = wq.view(out_features, in_features)
        packed_w = (wq[:, ::2] << 4) | wq[:, 1::2]
        
        # Create new module
        new_module = cls(
            in_features, out_features, 
            bias=linear_module.bias, 
            group_size=group_size, 
            device=device, 
            dtype=dtype
        )
        new_module.packed_weight.copy_(packed_w)
        new_module.scale.copy_(scale.view(out_features, -1))
        new_module.zero_point.copy_(zp.view(out_features, -1))
        
        return new_module

    def forward(self, x):
        # Unpack uint8 to float32 first for precision
        # Converting directly to float16 here can cause overflow during later arithmetic
        high = (self.packed_weight >> 4).to(torch.float32)
        low = (self.packed_weight & 0x0F).to(torch.float32)
        
        # Interleave high and low bits to reconstruct (out_features, in_features)
        wq = torch.stack([high, low], dim=-1).view(self.out_features, self.in_features)
        
        # Expand scales and zero points to match group_size (reconstruct in float32)
        s = self.scale.unsqueeze(-1).expand(-1, -1, self.group_size).reshape(self.out_features, self.in_features).to(torch.float32)
        z = self.zero_point.unsqueeze(-1).expand(-1, -1, self.group_size).reshape(self.out_features, self.in_features).to(torch.float32)
        
        # Reconstruct weights: w = wq * s + z (all in float32)
        w = (wq * s) + z
        
        # Force float32 for the linear sum to prevent overflow during accumulation.
        # This is critical for numerical stability on Turing (RTX 20-series) GPUs.
        res = F.linear(x.float(), w, self.bias.float() if self.bias is not None else None)
        return res.to(x.dtype)

def replace_with_windows_int4(model, group_size=128, filter_fn=None):
    """Recursively replace Linear layers with WindowsInt4Linear using FQNs."""
    # We need a list of replacements to avoid modifying while iterating
    replacements = []
    for fqn, module in model.named_modules():
        if isinstance(module, nn.Linear):
            if filter_fn is None or filter_fn(module, fqn):
                replacements.append((fqn, module))
    
    for fqn, old_module in replacements:
        logger.info(f"[Quant] Replacing {fqn} with WindowsInt4Linear")
        
        # Find parent
        parts = fqn.split(".")
        parent = model
        for part in parts[:-1]:
            parent = getattr(parent, part)
        
        # Replace
        new_module = WindowsInt4Linear.from_linear(old_module, group_size=group_size)
        setattr(parent, parts[-1], new_module)
