# TP Linux
## TP 1 - Prise en main de la carte VEEK-MT2S
### 1.1 Préparation de la carte SD
### 1.2 Démarrage
Utilisation PC + VM. On utilise une machine virtuelle (distribution debian GNU/Linux) pour compiler les fichiers qu'on va mettre sur la carte Soc car elle est plus puissante que la carte Soc et donc c'est plus rapide. 

Carte SoC connectée ds un premier au PC en liaison série puis en ssh

### 1.3.2 Utilisez un logiciel de liaison série
Comme je n'ai pas Putty, j'ai utilisé TeraTerm.   
Configuration : baud rate __115200__  
![image](https://github.com/Chatvolant/TP_Linux/assets/143954035/bfb65803-e2da-4550-9039-b0736bede5bd)

Taille occupée : 1.3G +9.8M+4K ≈ 1.31 G alors que notre carte SD a un volume de stockage de 16Go
On reboot le système et on exécute la commande ./resize2fs_once.

![image](https://github.com/Chatvolant/TP_Linux/assets/143954035/8dce3ac6-9a28-4c28-8984-a1b549585502)
![image](https://github.com/Chatvolant/TP_Linux/assets/143954035/56b98a4a-9ba2-409d-995c-08fb28adbde3)
 
On a plus d'espace disponible maintenant : 
12G+0.375G+0.376G+0,366+0,005+0,376+0,076= 13,574 G dispo au total

### 1.3.3 Configuration réseau

![image](https://github.com/Chatvolant/TP_Linux/assets/143954035/e57fc1e1-794d-4523-b833-983f0bd4b478)     

Ifconfig après avoir modifié interfaces   
![image](https://github.com/Chatvolant/TP_Linux/assets/143954035/2e1f974c-8d1b-455a-a929-88c6cb64295b)  
L’@ IP a changé (le dernier nombre n’est plus le même)
Ensuite je connecte mon ordi au réseau D060 vu que la carte qu’on utilise est connectée dessus (via cable ethernet).
Je ping la carte avec mon PC
![image](https://github.com/Chatvolant/TP_Linux/assets/143954035/55d151d3-c1e5-4721-bc58-07bbcfcc3cee)

Pas de paquets perdus, tous ceux qui ont été envoyés sont reçus dont la carte est bien connectée sur le réseau associé au wifi D060     
Pour se connecter avec le PC en ssh à la carte    
![image](https://github.com/Chatvolant/TP_Linux/assets/143954035/0f57757c-e61b-4581-ad5b-9047dfe00aa2)

Il fallait depuis le début se mettre sur la machine virtuelle au lieu du PC ;_ ;
En fait on compile dans la VM et on envoie la cible dans la carte, où on va aussi exécuter le fichier objet .o quoi (« sortie » du compilateur)  

### 1.4 Découverte de la cible
#### 1.4.1 Exploration des dossiers /sys/class et /proc   

Fichiers cpuinfo, ioports, iomem    

__cpuinfo__ : donne des informations sur les processeurs utilisés dans le système  
__ioports__ : Fournit une liste des régions de ports actuellement enregistrées utilisées pour la communication en entrée ou en sortie avec un device.       
__iomem__ : montre la carte mémoire actuelle du système pour chaque physical device.      

![image](https://github.com/Chatvolant/TP_Linux/assets/143954035/3b320c56-1490-46de-8e6b-338a08201723)

![image](https://github.com/Chatvolant/TP_Linux/assets/143954035/34bf9a7c-1242-4f1f-9b15-0497ca491c64)

commande cat ioports donne rien bizarrement   
Pour sortir de la cmd less on tape « q »   

![image](https://github.com/Chatvolant/TP_Linux/assets/143954035/55260616-22ed-4295-8bce-ea9ed3a0ef4a)

Je pense que sur un pc classique on a pas les fichiers fpga    

![image](https://github.com/Chatvolant/TP_Linux/assets/143954035/3c82975f-dbb5-4ad4-bdf6-67a83dc636f2)

![image](https://github.com/Chatvolant/TP_Linux/assets/143954035/9c9ee703-071b-4019-b8d0-a40937abe9e5)

![image](https://github.com/Chatvolant/TP_Linux/assets/143954035/acc545be-1615-46f0-9afd-95c39b884bca)

Iomem -> System RAM, I2c, SPI, ethernet, flash, gpio, dma, timer…    
root c'est juste le nom du compte utilisateur (mais comme le nom ici c'est "root", ça veut dire qu'on est en mode superutilisateur)

#### 1.4.3 Hello world !

On a écrit dans le fichier helloworld un programme qui affiche un hello world mais finalement j’ai modifié le nom du fichier en hello (mv helloworld.c hello.c)   
![image](https://github.com/Chatvolant/TP_Linux/assets/143954035/238016f3-c89b-41b2-b98b-d28248eec782)    

On peut pas exécuter le hello.o dans la VM.

Commande pour copier l'exécutable directement vers la SoC en ssh : 

__scp chemin_sur_VM root@IP_DE_LA_CARTE_SOC:chemin_sur_SOC__

![image](https://github.com/Chatvolant/TP_Linux/assets/143954035/57c3a731-5027-427d-b6ae-945b89b2c62a)

la commande echo "1"> nom_du_ficher permet d'écrire 1 dans le fichier   
de base echo est utilisée pour afficher du texte (ici le texte c'est le caractère "1")   

![image](https://github.com/Chatvolant/TP_Linux/assets/143954035/5bf9d94a-1156-4453-9a27-a42931541998)

On l’envoie vers la carte Soc.   
On a bien les Leds qui s’allument puis s’éteignent  

















## TP2 -  Modules kernel
Utilité des modules? -> __servent à implémenter des pilotes (drivers) p.22 *cours 3 Linux* + à implémenter des services dont le code doit être exécuté avec des
droits privilégiés__


## TP3 - Device tree
qu'est-ce que c'est? à quoi ça sert?

# EN COURS D'AJOUT!!!!!!!!!!!!!!!!!!!!!!!!!!!
