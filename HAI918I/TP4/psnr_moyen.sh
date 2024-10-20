#!/bin/bash

# Vérification des arguments
if [ "$#" -ne 2 ]; then
    echo "Usage: $0 dossier_images image_moyenne"
    exit 1
fi

DOSSIER_IMAGES=$1
IMAGE_MOYENNE=$2
FICHIER_RESULTATS="psnr_results.txt"

# Vérification que le dossier d'images existe
if [ ! -d "$DOSSIER_IMAGES" ]; then
    echo "Le dossier $DOSSIER_IMAGES n'existe pas."
    exit 1
fi

# Initialisation des variables
psnr_total=0
compteur=0

# Supprimer le fichier des résultats s'il existe déjà
> "$FICHIER_RESULTATS"

# Boucle sur les images du dossier
for image in "$DOSSIER_IMAGES"/*.pgm; do
    if [ ! -f "$image" ]; then
        echo "Aucune image trouvée dans le dossier."
        continue
    fi

    # Appeler le programme PSNR et capturer la sortie
    psnr_output=$(./PSNR "$image" "$IMAGE_MOYENNE")

    # Vérifier si le programme PSNR a réussi
    if [ $? -ne 0 ]; then
        echo "Erreur lors du calcul du PSNR pour l'image $image"
        continue
    fi

    # Extraire la valeur du PSNR de la sortie
    psnr_value=$(echo "$psnr_output" | grep -oP 'PSNR: \K[0-9]+(\.[0-9]+)?')

    # Vérifier si la valeur du PSNR a été extraite correctement
    if [ -z "$psnr_value" ]; then
        echo "Erreur lors de l'extraction de la valeur PSNR pour l'image $image"
        continue
    fi

    # Ajouter à la somme totale et incrémenter le compteur
    psnr_total=$(echo "$psnr_total + $psnr_value" | bc)
    compteur=$((compteur + 1))

    # Écrire le PSNR dans le fichier de résultats
    echo "PSNR pour $image : $psnr_value" >> "$FICHIER_RESULTATS"
done

# Calculer le PSNR moyen
if [ "$compteur" -gt 0 ]; then
    psnr_moyen=$(echo "scale=2; $psnr_total / $compteur" | bc)
    echo "PSNR moyen : $psnr_moyen" >> "$FICHIER_RESULTATS"
else
    echo "Aucune image valide n'a été traitée."
fi

echo "Les résultats PSNR ont été enregistrés dans $FICHIER_RESULTATS"

