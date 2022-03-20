# AISE_PROJECT_2022


    Mon projet est un programme qui permet d'analyser un processus en cours d'exécution, les fonctionnalités sont proche d'un débugger classiques
# Installation du projet


    $ git clone https://github.com/Hmina97/ProjetAISE.git


# Compilation

    $ make all 

# Execution de notre programme:
 
    $ ./debug_m fich_test addr(en hexa)

# Explication:
    Mon programme permet de faire des breakpoints, remonter la pile à l'aide d'une fonction do_backtrace, il a également un gestionnaire de signaux. 
    Toutes les fonctionalités se trouvent dans l'executable
    
# Diffcultés reconcontrés

  j'ai rencontré des grosses difficultés avec la fonction backtrace qui n'affiche pas le résultat attendu,j'ai pris énormement de temps à implémenter les breakpoints 
  mais ceci sont fonctionnels. 
    
