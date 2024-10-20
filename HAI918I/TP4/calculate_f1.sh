#!/bin/bash

# Vérification des paramètres
if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <fichier_scores.dat> <true_class>"
    exit 1
fi

fichier_scores="$1"
true_class="$2"

# Initialisation des compteurs pour les TP, TN, FP et FN
TP=0
TN=0
FP=0
FN=0

# Lecture du fichier de scores
while read -r line; do
    # Récupération des scores et de la classe prédite
    class=$(echo "$line" | awk '{print $1}')  # Classe 1
    score1=$(echo "$line" | awk '{print $2}')  # Classe 1
    score2=$(echo "$line" | awk '{print $3}')  # Classe 2

    # Vérification du format des scores
    if ! [[ $score1 =~ ^-?[0-9]+(\.[0-9]+)?$ ]] || ! [[ $score2 =~ ^-?[0-9]+(\.[0-9]+)?$ ]]; then
        echo "Erreur de format dans la ligne : $line"
        continue  # Passer à la ligne suivante
    fi

    # Prédire la classe basée sur les scores
    if (( $(echo "$score1 > $score2" | bc -l) )); then
        predicted_class=1
    else
        predicted_class=2
    fi

    # Mise à jour des TP, TN, FP et FN
    if [ "$predicted_class" -eq "$true_class" ]; then
        if [ "$true_class" -eq "$class" ]; then
            ((TP++))  # True Positive
        else
            ((TN++))  # True Negative
        fi
    else
        if [ "$true_class" -eq "$class" ]; then
            ((FN++))  # False Negative
        else
            ((FP++))  # False Positive
        fi
    fi
done < "$fichier_scores"

# Calcul des métriques
if (( TP + FP > 0 )); then
    precision=$(echo "scale=4; $TP / ($TP + $FP)" | bc)
else
    precision=0
fi

if (( TP + FN > 0 )); then
    recall=$(echo "scale=4; $TP / ($TP + $FN)" | bc)
else
    recall=0
fi

# Calcul de l'F1-score
if [[ $(echo "$precision + $recall" | bc) != "0" ]]; then
    f1_score=$(echo "scale=4; 2 * $precision * $recall / ($precision + $recall)" | bc)
else
    f1_score=0
fi

# Affichage des résultats
echo "TP: $TP, TN: $TN, FP: $FP, FN: $FN"
echo "Précision: $precision"
echo "Rappel: $recall"
echo "F1-score: $f1_score"

