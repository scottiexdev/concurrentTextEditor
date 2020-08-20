# concurrentTextEditor
Concurrent text editor project. 

Naming conventions
 - Types start with capitals: MyClass - MyTemplate - MyNamespace
 - functions and variables start with lower case: myMethod
 - constants are all capital: const int PI=3.14159265358979323;
 - private member variables start with  "_" e.g: private: int _x;

Practical coding notes:
 - Usare il passaggio delle variabile per reference e non i puntatori: se passiamo per reference non dobbiamo preoccuparci che il puntatore possa avere valore nullo: si scatena un'eccezione automaticamente
 
TODO's:
 - QDialog popup lato client se il login fallisce - done
 - QDialog popup lato client se il signup fallisce - done
 - Visualizzare lista file - done
 - Bottone open che chiama funzione per aprire il file -done
 - Funzione che richiede il file - done 
 - Funzione che manda il file - done
 - Finestra con text editor all'apertura di un file - done
 - Arricchire il json che manda la lista dei file con piu' info sul file (ex owner, created etc)
 
 Note:
 - la cartella IconsBar va inserita dentro "build-concurrentTextEditor-Desktop[...]-Profile"


Git from command line:
1. clone repo
2. git pull origin master
3. git branch <new_branch_name>
4. git checkout <new_branch_name>
5. Fare modifiche e quel si deve fare
6. git add <file_modificato_1> <file_modificato_2>    ------ per vedere i file che sono stati cambiati fare "git status"
7. Dopo aver controllato con git status quali file sono pronti per essere committati si procede a commiattare con: git commit -m "commit message"
8. Push dei cambiamenti sul proprio branch: git push origin <new_branch_name>
