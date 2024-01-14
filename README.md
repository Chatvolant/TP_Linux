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

__Taille occupée__ : 1.3G +9.8M+4K ≈ 1.31 G alors que notre carte SD a un volume de stockage de 16Go
On reboot le système et on exécute la commande ./resize2fs_once.

![image](https://github.com/Chatvolant/TP_Linux/assets/143954035/8dce3ac6-9a28-4c28-8984-a1b549585502)
![image](https://github.com/Chatvolant/TP_Linux/assets/143954035/56b98a4a-9ba2-409d-995c-08fb28adbde3)
 
On a plus d'espace disponible maintenant : 
12G+0.375G+0.376G+0,366+0,005+0,376+0,076= 13,574 G dispo au total

### 1.3.3 Configuration réseau

![image](https://github.com/Chatvolant/TP_Linux/assets/143954035/e57fc1e1-794d-4523-b833-983f0bd4b478)     

ifconfig après avoir modifié interfaces   
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

commande __cat ioports__ donne rien bizarrement   
Pour sortir de la cmd less on tape « q »   

![image](https://github.com/Chatvolant/TP_Linux/assets/143954035/55260616-22ed-4295-8bce-ea9ed3a0ef4a)

Je pense que sur un pc classique on a pas les fichiers fpga    

![image](https://github.com/Chatvolant/TP_Linux/assets/143954035/3c82975f-dbb5-4ad4-bdf6-67a83dc636f2)

![image](https://github.com/Chatvolant/TP_Linux/assets/143954035/9c9ee703-071b-4019-b8d0-a40937abe9e5)

![image](https://github.com/Chatvolant/TP_Linux/assets/143954035/acc545be-1615-46f0-9afd-95c39b884bca)

__Iomem__ -> System RAM, I2c, SPI, ethernet, flash, gpio, dma, timer…    
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

## TP2 -  Modules kernel (20/11/2023)
Adresse IP du SoC aujourd'hui : 192.168.88.72

### 2.1 Accès aux registres

En mode utilisateur on peut certes écrire/lire dans les registres en passant par la fonction mmap (remappe l'adresse physique des drivers en adresse virtuelle) mais on aura pas la possibilité d’interagir avec les interruptions. Pas de moyen d'être interrompu donc pose problème quand on écrit sur un driver. Or, pour un système, le fait de pouvoir être interrompu est nécessaire à son bon fonctionnement (signaux d'erreurs, événements matériels) D'où l'utilisation des modules.  

Utilité des modules? -> __servent à implémenter des pilotes (drivers) + à implémenter des services dont le code doit être exécuté avec des
droits privilégiés (Sources : p.22 *cours 3 Linux*)__

On a allumé la led la plus à gche. 
Si on met 3 à la place de 1, la led à gauche de la led précédente s’allume aussi (deux LEDs allumées)
![image](https://github.com/Chatvolant/TP_Linux/assets/143954035/9dc3b30d-b8f5-436f-a6a6-d5b09a14a447)  
1<<6 Allume la led 6

### 2.2 Compilation de module noyau sur la VM


![image](https://github.com/Chatvolant/TP_Linux/assets/143954035/984a4a5c-c198-42a4-bbc1-8faa7f2e494a)  

Dmesg permet de nous retourner le printk (printkernel)   

![image](https://github.com/Chatvolant/TP_Linux/assets/143954035/6113d7e9-7945-4573-9a25-057e1b0c1072)   

Décharger le module du noyau :  

![image](https://github.com/Chatvolant/TP_Linux/assets/143954035/22d1ab4d-9b7d-417b-8e93-89a96df9d852)  

Note: on est obligé de se mettre en mode pour pouvoir exécuter la commande qui permet de décharger le module qu'on a chargé du noyau  

Fichier __.ko__ -> __kernel object__ (générés quand on a fait la commande make auparavant)
__Make clean__ -> pour __supprimer les fichiers__ qu’on a __généré__ avec la commande __make__ (makefile)    

![image](https://github.com/Chatvolant/TP_Linux/assets/143954035/0f402c1b-bd60-4cb4-baf5-62740d6ec36e)  

Avec les paramètres : 

![image](https://github.com/Chatvolant/TP_Linux/assets/143954035/28f9fe2a-d1eb-4a5b-b553-34edfa103006)   

![image](https://github.com/Chatvolant/TP_Linux/assets/143954035/baffac13-38ab-4f71-a863-e830fab9034c)  

__insmod__ charge dans le noyau le fichier __.ko__

![image](https://github.com/Chatvolant/TP_Linux/assets/143954035/7d69e317-55d0-4393-8c17-ed5683a79eee)  

![image](https://github.com/Chatvolant/TP_Linux/assets/143954035/d6eb8558-de6a-49c9-8ab5-822a0bf078f7)

![image](https://github.com/Chatvolant/TP_Linux/assets/143954035/4a12d059-0b4e-4fbe-b6dd-e9f29564d3de)   


![image](https://github.com/Chatvolant/TP_Linux/assets/143954035/ee32b881-dea0-4017-9c0b-fc195a438deb)

__Timer__

![image](https://github.com/Chatvolant/TP_Linux/assets/143954035/95d95470-3313-458d-bafc-3dd1d2576c2e)   

### 2.3 CrossCompilation de modules noyau

__Cross-compilation__ : permet de compiler des fichiers pour un autre type de système d'exploitation   
Ici on cross compile dans la VM pour la SoC

![image](https://github.com/Chatvolant/TP_Linux/assets/143954035/639a790d-879d-4ad0-89de-6885bf223f45)   

#### 2.3.1 Préparation de la compilation

![image](https://github.com/Chatvolant/TP_Linux/assets/143954035/1ce74d30-4afe-4c07-becf-2246837dcc13)  

![image](https://github.com/Chatvolant/TP_Linux/assets/143954035/9cb8d5d3-ec8d-4243-b0fe-38a35ede4325)   

#### 2.3.2 Récupération de la configuration actuelle du noyau

![image](https://github.com/Chatvolant/TP_Linux/assets/143954035/59b64769-9f6d-48a7-8e2f-3d9b3ac3afff)   

Quel est le rôle des lignes commençant par export ?   
- __chemin_arm-linux-gnueabihf-__ correspond à notre compilateur. Le train (à la fin) permet de s’assurer que tt ajout de préfixe comme gcc se fera correctement avec ce chemin  
- __export__ pour définir des variables d’environnement
- __CROSS_COMPILE__ -> pour définir l’outil de compilation quand on va faire compilation croisée
- __ARCH__ -> pour spécifier l’architecture cible lors de la compilation du noyau LINUX.

#### 2.3.3 Hello World

Séance 3 suite de 2.3.2 (27/11/2023)   
@de la SOC pdt cette séance : 192.168.88.28   

![image](https://github.com/Chatvolant/TP_Linux/assets/143954035/9a1aaa5b-982a-45d4-b05e-cee6d5f6b5fb)   
![image](https://github.com/Chatvolant/TP_Linux/assets/143954035/14a80883-1847-4dcc-addb-bdcf63115925)   
![image](https://github.com/Chatvolant/TP_Linux/assets/143954035/811b0646-4357-4c3f-87cc-d89b5711b771)   

__Attention : Il faut compiler avec les sources de ARM pour Intel !!!!__

![image](https://github.com/Chatvolant/TP_Linux/assets/143954035/bdfec787-0ea0-4ad3-84d8-2f8224149316)  

Linux DE10-Standard 4.5.0-00198-g6b20a29   

![image](https://github.com/Chatvolant/TP_Linux/assets/143954035/3d953516-649a-4ebf-aedf-972430d42538)   
![image](https://github.com/Chatvolant/TP_Linux/assets/143954035/6bae346b-72f3-4e1e-9d67-dfaa64365faa)   

Après avoir fait le make (qui a réussi), on transfert fichier vers la carte SoC (on a compilé avec ARM c’est pour ça qu’on utilise le kernel ds linux-socfpga dans notre fichier Makefile)

![image](https://github.com/Chatvolant/TP_Linux/assets/143954035/44115a2e-8960-40a6-9580-a56e8cbf7a4c)   
![image](https://github.com/Chatvolant/TP_Linux/assets/143954035/2beff51b-9bf8-4a70-a576-e8b17f58d8c4)  

#### 2.3.4 Chenillard (Yes !)

![image](https://github.com/Chatvolant/TP_Linux/assets/143954035/c2948f4b-5d4d-496d-980d-f203371833f4)   
![image](https://github.com/Chatvolant/TP_Linux/assets/143954035/7d04ce1e-698d-4e34-96a9-c7da1d50c009)   

Module chenille dans /proc (jsplus mais jcrois que c’est où on met les modules chargés en noyau)
On a bien réussi à modifier Pattern et speed :   

![image](https://github.com/Chatvolant/TP_Linux/assets/143954035/37c599c9-79fd-4a1e-ac93-3f654ba1f818)


## TP3 - Device tree

04/12/2023
@ IP de la Soc : 192.168.88.102

__Qu'est-ce que c'est?__ (cours 4, p.11)

- Structure de données décrivant les périphériques d’une machine
- Utilisé par le noyau (Linux ou autre)
- Décrit le(s) CPU(s), la mémoire, les bus et les périphériques.

__Objectif du TP__ :  
- définir son propre périphérique
- programmer un module qui identifie la présence du périphérique et se configure automatiquement en sa présence. Cet automatisme s’appuye sur le Device Tree.

 On veut accéder à la partition de boot sur la carte VEEK (SoC)  
 "Montage" de la partition : 

 ![image](https://github.com/Chatvolant/TP_Linux/assets/143954035/2d76cc91-5a74-44ee-a4ab-1c06177c20cf)   

 Device Tree après avoir fait correctement ce qui était demandé :  

 ![image](https://github.com/Chatvolant/TP_Linux/assets/143954035/c473b567-a62f-4e6e-a686-080a44afd125)

### 3.1 Module accedant au LED via /dev
Sur cette partie il fallait récupérer le fichier gpio-leds.c sur moodle et le compiler. Pb : On a pas réussi à le compiler au début. On s'est ensuite rappelé qu'il fallait en fait cross-compiler.

1. ![image](https://github.com/Chatvolant/TP_Linux/assets/143954035/7eb64e77-c2e0-412d-b853-6a7d13542694)
2. Faire en sorte de placer dossier qu’on va compiler en dehors du dossier src (dossier partagé entre la VM et le PC)
3. On fait make en utilisant ce makefile (on l’avait utilisé pour le chenillard, on fait bien attention à changer le nom pour ça colle avec celui du fichier .c qu’on va compiler)  
![image](https://github.com/Chatvolant/TP_Linux/assets/143954035/1c19b14b-6d6f-4c86-91b8-ec53ef00c5f5)


__Fonction read__ : pour lire l’état des LEDS   
__Fonction write__ : ecrire l’état des LEDs   
__Remove__ : pour retirer du système le périphérique géré par le driver   
__Probe__ : appelée quand le noyau (kernel) trouve un nouveau device que notre driver peut gérer (dans notre cas, elle doit être appelée seulement pour 1 instantiation du module Ensea LEDS)   

Rôle quand ces fonctions rentrent en action :  pour initialiser le misc et configurer le périphérique   

@IP Soc : 192.168.88.121   

Gpio-leds   
![image](https://github.com/Chatvolant/TP_Linux/assets/143954035/d52453c8-f749-406a-9eb6-67f290b633e7)   


### 3.2 Module final

#### 3.2.1 Cahier des charges - Chenillard (non réussi)

![image](https://github.com/Chatvolant/TP_Linux/assets/143954035/4beba80c-fa89-4ead-8848-48887a52f988)    
6 et 9 ne sont pas pris en numero donc on choisit ces numeros pour le numero de majeur et de mineur (on va créer fichier driver dans dossier dev comme dans le TD 3   

