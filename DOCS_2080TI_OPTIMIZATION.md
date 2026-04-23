# Guide d'Optimisation Technique : ACE-Step 1.5 XL sur RTX 2080 Ti

Ce document détaille les modifications techniques apportées à ce fork pour permettre l'exécution du modèle **ACE-Step 1.5 XL (4B parameters)** sur une configuration matérielle grand public : **NVIDIA RTX 2080 Ti (11Go VRAM)** et **32Go de RAM** sous Windows.

Bien que le code puisse paraître "brouillon" par endroits, ces ajustements sont le résultat d'une recherche de stabilité numérique et d'optimisation de la mémoire vidéo (VRAM) pour l'architecture **Turing**.

---

## 1. Le Défi Technique : 4B Params vs 11Go VRAM

Le modèle ACE-Step 1.5 XL pèse environ **9 Go** en précision `bfloat16`. Une fois chargé, il ne laisse quasiment aucune place pour :
- Le **VAE** (~0.3 Go)
- Le **Text Encoder** (~1.2 Go)
- Les **Activations** lors de l'inférence (qui augmentent avec la durée de la musique)
- Le **Contexte CUDA** (~0.5 Go)

Sur une carte de 11 Go, l'out-of-memory (OOM) est systématique sans optimisations lourdes.

---

## 2. Quantification 4-bit (INT4 Weight-Only)

### La Problématique Windows
La bibliothèque `torchao` (utilisée pour la quantification) s'appuie sur le backend `mslk` pour ses noyaux INT4 optimisés. Ce backend est actuellement exclusif à Linux. Sur Windows, `torchao` bascule sur des chemins non optimisés extrêmement lents (jusqu'à 140s par étape de diffusion).

### La Solution : `WindowsInt4Linear`
Nous avons implémenté une couche de compatibilité personnalisée (`acestep/core/generation/handler/windows_int4_linear.py`) qui :
1. **Stockage Packé** : Compresse deux poids de 4-bit dans un seul octet `uint8`.
2. **Quantification Asymétrique par Groupe** : Utilise une `group_size=128` pour maintenir une fidélité sonore proche du modèle original.
3. **Optimisation Inductor** : Le noyau de décompression est écrit en PyTorch pur (opérations bitwise) pour permettre à `torch.compile` de fusionner la déquantisation directement dans le produit matriciel (kernel fusion).

**Résultat** : Une vitesse d'exécution ~4.5x plus rapide que le mode non-patché sur Windows (~31s/step vs 140s/step).

---

## 3. Stabilité Numérique sur Architecture Turing

### Le problème des NaNs (Not a Number)
La RTX 2080 Ti (Compute Capability 7.5) ne supporte pas nativement le format `bfloat16`. En utilisant `float16`, le modèle XL subit des débordements numériques (overflows) fréquents dans les couches de self-attention et lors de l'accumulation des produits matriciels de grande taille. Cela se traduit par des sorties audio "silencieuses" ou des erreurs de type NaNs.

### Solutions de Stabilité
1. **Accumulation en Float32** : Dans notre noyau `WindowsInt4Linear`, le calcul de la somme pondérée est explicitement forcé en `float32` avant d'être reconverti dans le type du modèle.
2. **Forçage SDPA** : Nous désactivons `FlashAttention` (souvent instable ou non supporté sur Turing pour les très longues séquences) au profit de `Scaled Dot Product Attention` (SDPA) de PyTorch avec un correctif de stabilité forcée.
3. **Contrôle du DType** : Possibilité de forcer l'intégralité du pipeline en `float32` via la variable d'environnement `ACESTEP_DTYPE=float32` au prix d'une consommation VRAM accrue.

---

## 4. Configuration Recommandée (.env)

Pour reproduire ces résultats sur une 2080 Ti, votre fichier `.env` doit contenir :

```bash
# Activation de la quantification 4-bit pour faire tenir le modèle XL
ACESTEP_QUANTIZATION_MODE=int4_weight_only

# Indispensable pour la performance du noyau custom INT4
ACESTEP_COMPILE_MODEL=true

# Stabilité numérique (si vous rencontrez des NaNs avec float16)
# ACESTEP_DTYPE=float32  # Optionnel : augmente l'usage VRAM mais garantit la stabilité

# Augmentation du timeout pour les longues générations sur matériel plus lent
ACESTEP_GENERATION_TIMEOUT=36000
```

---

## 5. Benchmarks (Indicateurs réels)

*Configuration : RTX 2080 Ti 11GB, Windows 11, ACE-Step 1.5 XL Turbo.*

| Mode | Usage VRAM (Peak) | Temps / Étape | Qualité Audio |
| :--- | :--- | :--- | :--- |
| **Standard (BF16)** | > 12 GB (OOM) | N/A | N/A |
| **INT8 (Quantized)** | ~8.5 GB | ~30s | Excellente |
| **INT4 (ACE-Step Fork)** | **~6.8 GB** | **~31s** | **Très Bonne** |

---

## 6. Pourquoi ce Fork ?

L'objectif est de prouver qu'il n'est pas nécessaire de posséder une H100 ou une RTX 4090 pour expérimenter avec les modèles de génération musicale les plus pointus. En acceptant une légère dégradation de la vitesse au profit de l'accessibilité, nous permettons à une plus large communauté de créateurs d'utiliser ACE-Step 1.5 XL.

*Note : Le code contient des hacks spécifiques à Windows et à l'architecture Turing qui ne sont pas forcément élégants, mais qui "font le job".*

---
*Documentation rédigée pour le Fork ACE-Step 1.5 XL Optimization.*
