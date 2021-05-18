Pour récuperer localement les modifications en lignes. Ca override les fichiers locaux qui sont déjà sur Github mais ne touche pas ceux qui n'y sont pas

$ *git pull*

Pour voir les fichiers modifiés/ajouté localement. Il y a les fichiers suivis (tracked) et les fichiers non suvis (untracked) qui sont typiquement les nouveaux fichiers.

$ *git status*

Pour préparer un fichier à être synchronisé:

$ *git add path_du_fichier*

Pour préparer tous les fichiers suivis à être synchronisés:

$ *git add -u*

Note: dans le path_du_fichier, "\*" permet de remplacer n'importe quel groupe de lettres ("*obj.\**" selectionne tous les fichiers qui commencent par "*obj.*").
Les fichiers préparés pour synchronisation apparaîtrons vert dans *git status*. Ca vaut la peine de vérifier.

Pour synchroniser les fichiers localement:

$ *git commit -m "message_de_synchro"*

Après avoir *commit*, synchroniser en ligne sur GitHub avec

$ *git push*



Exemples typiques:

Créer et ajouter un nouveau fichier au projet:

*créer fichier*<br>
$ *git add path_du_fichier*<br>
$ *git commit -m "Creation de nomdufichier"*<br>
$ *git push*

Synchroniser toutes les modifications de fichier déjà créés (il faut éviter de le faire sans avoir travaillé sur les fichiers les plus récents):

$ *git add -u*<br>
$ *git commit -m "Nom de modification"*<br>
$ *git push*
