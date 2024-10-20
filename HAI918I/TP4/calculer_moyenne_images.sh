#!/bin/bash

# Vérification du nombre d'arguments
if [ "$#" -ne 2 ]; then
    echo "Usage: $0 dossier_images fichier_sortie_moyenne"
    exit 1
fi

# Assignation des variables des arguments
DOSSIER_IMAGES=$1
FICHIER_SORTIE_MOYENNE=$2

# Vérifier que le dossier d'images existe
if [ ! -d "$DOSSIER_IMAGES" ]; then
    echo "Le dossier $DOSSIER_IMAGES n'existe pas."
    exit 1
fi

# Compteur et fichiers temporaires
COMPTEUR=0
FICHIER_TEMPORARY="temp_sum.pgm"

# Parcourir toutes les images dans le dossier et les additionner pixel par pixel
for IMAGE in "$DOSSIER_IMAGES"/*.pgm; do
    if [ "$COMPTEUR" -eq 0 ]; then
        # Si c'est la première image, initialiser le fichier temporaire avec cette image
        cp "$IMAGE" "$FICHIER_TEMPORARY"
    else
        # Additionner l'image courante à l'image temporaire
        convert "$FICHIER_TEMPORARY" "$IMAGE" -compose plus -composite "$FICHIER_TEMPORARY"
    fi

    # Incrémenter le compteur
    COMPTEUR=$((COMPTEUR + 1))
done

# Vérifier si on a traité au moins une image
if [ "$COMPTEUR" -eq 0 ]; then
    echo "Aucune image trouvée dans le dossier $DOSSIER_IMAGES."
    exit 1
fi

# Calculer la moyenne en divisant chaque pixel de l'image temporaire par le nombre total d'images
convert "$FICHIER_TEMPORARY" -evaluate divide $COMPTEUR "$FICHIER_SORTIE_MOYENNE"

# Supprimer le fichier temporaire
rm "$FICHIER_TEMPORARY"

echo "Image moyenne calculée et sauvegardée dans $FICHIER_SORTIE_MOYENNE."

