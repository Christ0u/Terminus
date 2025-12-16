# Compte rendu

Terminus est un interpréteur de commandes shell Linux implémenté en C.

Ce compte rendu détaille son principe de fonctionnement global ainsi que certains aspects plus spécifiques tel que la logique utilisée pour l'implémentation des différents opérateurs, par exemple.

## I - Boucle REPL

La boucle REPL (Read, Evaluate, Print, Loop) est une méthode d'interraction avec un programme informatique particulièrement adaptée à l'implémentation d'un terminal.

Elle se compose de trois séquences répétées indéfiniement jusqu'à la sortie du programme :

### > Lecture (Read)

Tout d'abord, l'utilisateur saisit une série d'instructions dans une invite de commandes. Une fois validée (touche `Enter`), les instructions sont lues pas le programme.

### > Evaluation (Evaluate)

Ensuite, les instructions sont décomposées en un tableau de jetons.

Par exemple, l'utilisateur saisit l'entrée suivante :

```BASH
ls -l | grep toto
```

Chaque élément (jeton) composant cette suite d'instructions est décomposée de la manière suivante : `["ls", "-l", "|", "grep", "toto"]`.

Une fois les jetons isolés, chacun d'entre eux est interprété et déclenche une action bien définie. Par exemple, le caractère `|` crée un tube (voir chapitre IV).

### > Affichage (Print)

Le résultat des opérations est affiché dans le terminal. Le programme peut également afficher des messages d'erreurs dans le cas d'instructions erronées.

## II - Définition d'une commande

La structure command_t sert à représenter une commande saisie par l’utilisateur dans notre shell. Je voulais dans un premier temps utiliser une classe, mais comme nous sommes en C et non en C++, je me suis rabattue sur une structure.

Elle contient un tableau argv qui stocke la commande et ses arguments. Les champs redir_out, redir_append, redir_in et heredoc indiquent si la commande contient des redirections (>, >>, < ou <<). 

Les champs outfile, infile et heredoc_content stockent les fichiers ou le texte associés à ces redirections. heredoc_delimiter mémorise le mot de fin d’un here-document, et background indique si la commande doit s’exécuter en arrière-plan avec &.

Par exemple, pour la commande : 
```bash
    echo hello > fichier.txt &
```
on aura :

- argv = ["echo", "hello", NULL]

- redir_out = 1 et outfile = "fichier.txt"

- background = 1

Cette structure permet de centraliser toutes les informations nécessaires pour exécuter la commande correctement, que ce soit un builtin ou une commande externe.

## III - Commandes "built-in"

<!--
On détaille ici comment on fait la différence entre une builtin et une commande bash native
-->

Les commandes dites "built-in" sont des commandes internes qui ont leur propre implémentation au sein du code.

Terminus embarque les commandes built-in suivantes :

- `cd` : Change le répertoire de travail du shell
- `pwd` : Affiche le nom du répertoire de travail courant
- `echo` : Écrit les arguments sur la sortie standard
- `exit` : Termine le shell

Une distinction doit donc être faite entre ces commandes built-in intégrée à Terminus et les commandes natives Bash.

Pour ce faire, nous nous appuyons sur la structure `buitlin` qui, à un nom, associe une fonction.

Une boucle parcourt l'ensemble des built-in enregistrées. Si la saisie utilisateur correspond à l'une d'entre-elle, la fonction associée est alors exécutée. En revanche, si la saisie utilisateur ne correspond à aucune built-in enregistrée, elle est intérprétée comme étant une commande native Bash.

## IV - Opérateurs

<!--
Un ptit point sur les opérateurs serait pas de refus je pense.
- Opérateurs de redirection : < << > >> |
- Opérateurs de contrôle : && || &
-->

Dans notre shell, les **opérateurs** permettent de contrôler le flux des commandes ou de gérer les fichiers.  

- **Opérateurs de redirection :**
  - `<` : redirige l'entrée depuis un fichier (`commande < fichier`).
  - `<<` : here-document, permet de donner directement un texte à une commande (`commande << EOF ... EOF`).
  - `>` : redirige la sortie vers un fichier en écrasant (`commande > fichier`).
  - `>>` : redirige la sortie vers un fichier en ajoutant à la fin (`commande >> fichier`).
  - `|` : crée un pipe pour envoyer la sortie d'une commande vers l'entrée d'une autre (`commande1 | commande2`).

- **Opérateurs de contrôle :**
  - `&&` : exécute la commande suivante seulement si la précédente a réussi.
  - `||` : exécute la commande suivante seulement si la précédente a échoué.
  - `&` : exécute la commande en arrière-plan.

Par exemple :  
```bash
echo "Salut" > message.txt &
```
Ici, > redirige la sortie vers message.txt et & indique que la commande s’exécute en arrière-plan.

Dans le code, chaque opérateur est géré via des fonctions spécifiques : builtin_redirect_output pour >/>>, builtin_redirect_input pour <, et builtin_heredoc_input pour <<. 

Les opérateurs de contrôle comme & sont pris en compte dans le champ background de la structure command_t.

## Conclusion

Océane : Ce projet était plutôt cool et fun à faire ! J'avais de l'appréhension au début car il fallait le faire en C et je ne suis pas très douée en systèmes. Le projet m'a pris plus de temps que prévu mais j'ai pu apprendre pas mal de choses. 

J'ai utilisé l'IA non pas pour coder mais pour qu'elle m'explique des choses, notamment sur les pointeurs et les adresses car malgré les cours de Julien Schnell et de Julien Haristoy je n'avais toujours pas compris comment cela fonctionnait. 

Je n'ai pas rencontré de seg fault (youpi) mais j'ai rencontré des core dumped (pas youpi). Au final, c'était juste moi qui oubliait d'include certains fichiers dans mon code.

Preuve : 

![preuve](https://i.ibb.co/n8rwvgh9/Capture-d-cran-du-2025-12-16-21-49-44.png)


Conclusion : 
![meme](https://i.ibb.co/bgJ38cf5/hehe-not-hehe.jpg)