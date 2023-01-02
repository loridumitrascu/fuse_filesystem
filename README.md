# Secured Ramdisk FileSystem With FUSE

## Rularea programului
 Pentru a rula programul, se va rula scriptul ```run.sh``` care prezintă următoarele opțiuni:

&emsp; 1.  ```-i``` - Instalează dependențele programului. 

&emsp; 2.  ```-c``` - Compilează programul.

&emsp; 3.  ```-m <mount_point> <disk_image_name>``` - Montează imaginea sistemului de fișiere în folderul dat &emsp; &emsp; ca mount point. Dacă folderul sau imaginea nu există, acestea se vor crea automat. 

&emsp; 4.  ```-u <mount_point>``` - Demontează sistemul de fișiere.

&emsp; 5.  ```-d <mount_point> <disk_image_name>``` - Va monta sistemul de fișiere în modul debug.

&emsp; 6.  ```-x <mount_point> <disk_image_name>``` - Va face clean-up aplicației.
 
 ## Features - Operațiile disponibile în sistemul de fișiere
 
Sistemul de fișiere  poate realiza următoarele funcționalități:

&emsp; • Crearea și ștergerea unui director;

&emsp; • Crearea, ștergerea, citirea, scrierea într-un fișier de dimensiuni foarte mari;

&emsp; • Trunchierea unui fișier (modificarea dimensiunii unui fișier);

&emsp; • Acceptă metadata (permisiuni și timestamps);

&emsp; • Deschiderea și închiderea unui fișier;

&emsp; • Listarea fișierelor dintr-un anumit director sau de la rădăcină;

&emsp; • Crearea de subdirectoare;

&emsp; • Redenumirea unui fișier;

&emsp; • Copierea și mutarea unui fișier;

&emsp; • Hardlinks & symlinks;

&emsp; • Stocare persistentă - structura sistemului de fișiere va fi păstrată după demontare;

  ## Dependente
  Pentru a putea rula programul, este nevoie de biblioteca libfuse-dev. Ea se poate instala folosind scriptul pus la dispoziție cu opțiunea ```-i ``` sau folosind următoarele comenzi:

```
sudo apt-get update
sudo apt-get install libfuse-dev
```
  ## Detalii de implementare
  
  Detaliile de implementare sunt precizate in mod amănunțit în ```Documentation/README.md```
