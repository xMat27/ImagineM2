#!/bin/bash

# Définir les chemins d'accès aux images
image_fixe="image_moyenne_classe1.pgm"
output_file="psnr_results.txt"

# Créer ou vider le fichier de résultats
echo "Image, PSNR" > "$output_file"

# Boucle sur les images dans le dossier imgFrog
for image_variable in imgFrog/*.pgm; do
    # Exécuter le programme PSNR et capturer sa sortie
    psnr_output=$(./PSNR "$image_variable" "$image_fixe")

    # Afficher la sortie pour déboguer (optionnel)
    echo "Calcul du PSNR entre $image_variable et $image_fixe : $psnr_output"

    # Écrire le résultat dans le fichier
    echo "$(basename "$image_variable"), $psnr_output" >> "$output_file"
done

# Afficher un message de fin
echo "Calculs terminés. Résultats sauvegardés dans $output_file."

