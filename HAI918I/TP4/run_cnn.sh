#!/bin/bash

# Vérification du nombre d'arguments
if [ "$#" -ne 2 ]; then
    echo "Usage: $0 dossier_images dossier_sortie programme_cnn"
    exit 1
fi

# Assignation des variables des arguments
DOSSIER_IMAGES=$1
DOSSIER_SORTIE=$2

# Création du dossier de sortie s'il n'existe pas
if [ ! -d "$DOSSIER_SORTIE" ]; then
    mkdir -p "$DOSSIER_SORTIE"
fi

# Initialiser un compteur pour limiter à 80 images
COMPTEUR=0

# Parcours des fichiers du dossier_images (limité à 80 fichiers)
for IMAGE in "$DOSSIER_IMAGES"/*.pgm; do
    if [ "$COMPTEUR" -ge 100 ]; then
        break
    fi
    
    # Nom de l'image sans le chemin
    NOM_IMAGE=$(basename "$IMAGE")

    # Exécution du programme CNN sur l'image
    echo "Traitement de $IMAGE..."
    ./CNN "$IMAGE" "$DOSSIER_SORTIE/$NOM_IMAGE"

    # Incrémentation du compteur
    COMPTEUR=$((COMPTEUR + 1))
done

echo "Traitement terminé. $COMPTEUR images traitées et sorties dans $DOSSIER_SORTIE."

