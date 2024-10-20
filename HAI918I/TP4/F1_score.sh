#!/bin/bash

g++ -o CNN CNN.cpp

# Vérifiez si le nombre d'arguments est correct
if [ "$#" -ne 2 ]; then
    echo "Usage: $0 chemin/vers/le/dossier_des_images true_class_label"
    exit 1
fi

# Répertoire contenant les images et classe réelle
image_dir="$1"
true_class_label="$2"
output_file="CNN.dat"

# Initialisation des compteurs
VP=0
FP=0
VN=0
FN=0

# Compteur pour le nombre d'images traitées
count=0

# Boucle à travers les fichiers d'images
for image in "$image_dir"/*.pgm; do
    # Limite à 80 images
    if [ "$count" -ge 80 ]; then
        break
    fi
    
    # Exécution du programme CNN
    ./CNN "$image"
    
    # Lecture des résultats du fichier CNN.dat
    # Récupérer la 2e et 3e colonne pour les classes 1 et 2
    read -a results <<< "$(tail -n 1 "$output_file")"
    class1_score="${results[1]}"
    class2_score="${results[2]}"
    
    # Prédiction basée sur les scores
    if (( $(echo "$class1_score > $class2_score" | bc -l) )); then
        predicted_class=1
    else
        predicted_class=2
    fi
    
    # Mise à jour des compteurs
    if [ "$predicted_class" -eq "$true_class_label" ]; then
        if [ "$predicted_class" -eq 1 ]; then
            VP=$((VP + 1))
        else
            VN=$((VN + 1))
        fi
    else
        if [ "$predicted_class" -eq 1 ]; then
            FP=$((FP + 1))
        else
            FN=$((FN + 1))
        fi
    fi
    
    count=$((count + 1))
done

# Calcul du F1-score
precision=$(echo "scale=4; $VP / ($VP + $FP)" | bc)
recall=$(echo "scale=4; $VP / ($VP + $FN)" | bc)
f1_score=$(echo "scale=4; 2 * ($precision * $recall) / ($precision + $recall)" | bc)

# Affichage des résultats
echo "Vrais Positifs (VP): $VP"
echo "Faux Positifs (FP): $FP"
echo "Vrais Négatifs (VN): $VN"
echo "Faux Négatifs (FN): $FN"
echo "F1-score: $f1_score"

