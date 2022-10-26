# Secured Ramdisk FileSystem With FUSE
## Overview
 <p align="justify">
  Proiectul își propune crearea unui sistem de fișiere folosind API-ul FUSE. Acesta reprezintă practic liantul dintre user-space și kernel-space și ne permite nouă, utilizatorilor neprivilegiați, să creăm un sistem de fișiere fără a modifica codul din kernel și de a lucra într-o manieră sigură.
 </p>
 <p align="justify">
   Pentru implementare, vom folosi un singur fișier mare ca emulator pentru un disc orientat pe blocuri folosit pentru stocare astfel încât să asigurăm persistența sistemului după demontare. Acest document explică arhitectura si organizarea aleasă, modul de legare a părților componente și observațiile relevante în dezvoltarea sistemului.
 </p>
  <p align="justify">
   În plus, pentru a mări gradul de securitate al aplicației, am decis ca sistemul nostru de fișiere să aibă toate datele criptate. Astfel, vom putea avea acces la fișiere numai prin intermediul filesystem-ului nostru căruia îi revine și rolul de a decripta datele citite și a le cripta la loc în momentul scrierii. Se va hotărî ulterior dacă vom permite doar unui utilizator privilegiat accesul la conținutul fișierelor sau dacă acest lucru se va realiza în momentul montării, dupa furnizarea unei parole.
 </p>
 
## Scopul Proiectului
   <p align="justify">
 Scopul nostru este de a realiza un sistem de fișiere simplu, rapid și eficient pentru stocarea și prelucrarea fișierelor și directoarelor, precum și obținerea unor cunoștințe practice despre elementele interne și conceptele necesare dezvoltării unui sistem de fișiere în Linux. 
   </p>
   
 ## Arhitectura si organizarea
  <p align="justify">
  Sistemul de fișiere va fi creat în memorie sub forma unei structuri arborescente sau folosind structuri de date specializate (superblocuri, inoduri, blocuri de date etc). În loc să citească și să scrie blocuri de disc, sistemul va folosi memoria principală pentru stocare (RAMDISK). Pentru ca datele să nu se piardă atunci când procesul s-ar termina și memoria ar fi eliberată, la demontare vom salva datele(metadatele și datele reale) pe disc printr-un singur fișier binar extern (disk image) care va fi citit și încărcat în memoria RAM la fiecare pornire a sistemului (la fiecare montare), actualizând structurile de date folosite în consecință. În cazul în care nu există un astfel de fișier, vom porni un sistem nou(gol). 
     </p>
     <p align="justify">
  Cum un sistem de fișiere controlează modul în care datele sunt stocate și preluate pentru prelucrare sau afișare, acesta trebuie să asigure o modalitate de a determina unde se oprește o informație și unde începe următoarea. Distingem astfel două aspecte importante:
      </p>
      <p align="justify">
  
  1. Structurile de date folosite: tipurile de structuri de pe disc utilizate de sistemul de fișiere pentru a-și organiza datele și metadatele (e.g. superblocurile, blocurile de date, inode-urile, bitmap-urile, arbori etc.);

  2. Accesul la date: cum se mapează apelurile efectuate de un proces pe structurile sale de date (cum manipulează diferite apeluri de sistem datele respective).

Astfel, pentru a obține un sistem funcțional, venim cu următoarele idei de implementare:
   </p>
   
### 1. Sistem de fișiere sub forma unui arbore
 <p align="justify">
În momentul în care sistemul de fișiere este montat, directorul rădăcină este creat (nodul rădăcină).  Fiecare nod va reprezenta un director sau un fișier regulat. De fiecare dată când trebuie să creăm un fișier/director, un nod este creat în arbore și acel nod este atașat părintelui corespunzător. Înainte de a realiza acest lucru, ar trebui verificată disponibilitatea nodului (dacă mai există suficient spațiu). În plus, fiecare fișier va avea asociat un inode în care se vor stoca metdatele despre acesta. 
</p>
<p align="justify">
 Nodul este structura de date folosită pentru a implementa structura arborescentă. Nodurile sunt structurile statistice pentru sistemul nostru de fișiere. Acestea stochează informații de bază despre fișier/director.
Pentru fiecare nod, va trebui să existe un inode care va conține informații despre:
 </p>
&emsp; • Mărimea fișierului în octeți;
 
&emsp; • Numărul de blocuri de date alocate în prezent acelui fișier (numai pentru fișierele obișnuite);

&emsp; • Ora ultimei modificări, creări și acces;

&emsp; • ID-ul ownerului și grupului (Permisiuni) etc.;

&emsp; • O înregistrare care conține informații despre numele fișierului, un indicator al tipului de fișier(director/regulat);

În cazul în care vom construi sistemul ca un arbore:

&emsp; • Un pointer către nodul părinte, un pointer către primul copil al nodului și un pointer către nodul care urmează nodului curent din directorul părinte.



### 2. Folosind blocuri și inode-uri
 <p align="justify">
 Sistemul de fișiere implică 5 blocuri majore - superblocul, lista de inoduri, bitmap pentru date și inode-uri și blocurile de date. Dimensiunea unui bloc de date ar fi de 4096 octeți (4K octeți) fiecare. În momentul în care sistemul de fișiere este montat, directorul rădăcină este creat. Inode-urile vor fi stocate într-o tabelă de inode-uri, fiecare conținând metadatele despre un fișier/director. Aceste metadate includ numărul inodului, tipul fișierului, numărul de linkuri, uid, gid, permisiunile asociate cu acest fișier etc. Pentru început, toate numerele inode-urilor sunt inițializate la 0. De asemenea, va trebui să reținem ce blocuri de date avem disponibile și câte există pentru fiecare fișier. Pentru a realiza acest lucru, la crearea unui nou fișier, vom verifica mai întâi bitmap-ul de date și pe cel de inode-uri pentru a găsi un bloc de date gol și un inod care este nealocat (care deține numărul de inod 0). Odată creat fișierul, acesta poate fi deschis pentru citire și scriere și vor fi actualizate structurie de date necesare. Când un fișier existent este recreat, acesta este trunchiat la dimensiunea 0 și tot conținutul său va trebui suprascris. Când un fișier este eliminat, tot conținutul acestuia este eliminat din blocul său de date, intrarea corespunzătoare acestui fișier este eliminată din directorul părinte, bitmap-ul este actualizat și conținutul inode-ului său este reinițializat.
  
 Începând cu blocul 0, aspectul sistemului de fișiere este descris după cum urmează:
 </p>

&emsp; &emsp; ![](../Assets/image1.png)

Unde: 

&emsp; • S reprezintă blocul rezervat superblocului;

&emsp; • i reprezintă blocul rezervat bitmap-ului pentru inode-uri;

&emsp; • d reprezintă blocul rezervat bitmap-ului pentru blocurile de date;

&emsp; • următoarele 5 reprezintă tabelul de inode-uri;

&emsp; • restul de blocuri sunt rezervate blocurilor de date.

**Superblocul:** va conține caracteristicile sistemului (dimensiunea sistemului, dimensiunea blocurilor, dimensiunea tabelei de inoduri, numărul maxim de inode-uri, dimensiunea bitmapului etc.).

**Bitmap-urile:** vor gestiona spațial, identificând dacă avem blocuri sau inode-uri libere.

**Inode-ul:** va stoca atributele, locațiile blocurilor pe disc, metadata despre fișier/director.

**Tabela de inode-uri:** o listă care conține inodurile din sistem. Conține un număr limitat de structuri de tip inode (numărul maxim de inode-uri înseamnă că vom avea un număr maxim de fișiere).

**Blocurile de date:** datele sunt salvate în blocuri (în general 1 block = 512 bytes). Fiecare inod face referire la unul sau mai multe blocuri de date din această zonă. Ele vor fi atribuite numai pentru fișiere, nu și pentru directoare.

În cazul în care vom construi sistemul folosind blocuri, inode-ul va avea următoarea structură:

&emsp; &emsp; ![](../Assets/image2.png)

## Dependențe de instalare/Constrângeri:
&emsp; • Instalarea bibliotecii libfuse

&emsp; • Sistemul de fișiere va fi construit pe o distribuție de Ubuntu.

## Avantajele folosirii arhitecturii 1:

&emsp; • Ușurință în implementare;

&emsp; • Nu se pune problema fragmentării.

## Avantajele folosirii arhitecturii 2:
&emsp; • Putem vedea cu ușurință câte inoduri și blocuri de date sunt libere analizând structura de date bitmap (o căutare constantă în timp);
&emsp; •  Cu fișiere mai mici avem acces foarte rapid la date. Pentru fișiere mai mari există șansa să se ocupe blocurile disponibile foarte repede.

## Ce operații vor fi disponibile în sistemul de fișiere?

Sistemul de fișiere ar trebui să poată realiza următoarele funcționalități:

&emsp; • Crearea și ștergerea un director;

&emsp; • Crearea, ștergerea, citirea, scrierea într-un fișier;

&emsp; • Trunchierea unui fișier (modificarea unui fișier);

&emsp; • Acceptă metadata (permisiuni și timestamps);

&emsp; • Deschiderea și închiderea un fișier;

&emsp; • Listarea fișierelor dintr-un anumit director sau de la rădăcină;

&emsp; • Crearea de subdirectoare;

&emsp; • Redenumirea unui fișier;

&emsp; • Stocare persistentă;

&emsp; • Asigurarea securității sistemului de fișiere prin criptarea acestuia.

## Instalarea bibliotecii libfuse-dev se va face astfel:

```
sudo apt-get update
sudo apt-get install libfuse-dev
```

