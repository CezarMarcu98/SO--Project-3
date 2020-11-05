Marcu Nicolescu Cezar
335CB

			Tema 3 - Loader de Executabil
		

Pentru inceput am captat seg fault-urile. Mi-am creat un handler cu ajutorul caruia sa gestionez apelurile de seg fault primite pe parcursul programului. Am verificat sa vad pe ce segment ma aflu cu un for ce merge pana la numarul de segmente. Daca adresa mea de pe care am primit seg fault-ul este continuta intre adresa de inceput (vaddr) a segmentului si adresa de sfarsit (vaddr + mem_size) inseamna ca sunt pe segmentul dorit. Daca nu gasesc nici un segment in care sa se incadreze adresa de fault , atunci propag la randul meu seg fault.
In cazul in care gasesc pe ce segment se incadreaza incep verificarile ulterioare pentru a-mi da seama ce fac in urma primirii acestui seg fault . Mai intai verific daca in memorie este deja mapata zona de pe care am primit seg fault : retin in segment->data un vector de adrese mapate si verific daca la pozitia paginii actuale a fost deja mapat ceva.

Aliniez segment->file_size cu page size (4096) pentru a putea sa imi dau seama cum ma raportez la pozitia la care sunt in momentul de fata in segment (aceasta aliniere imi va da un index ce imi spune inceputul ultimei pagini)

In functie de cum este rezultatul compararii dintre aliniere cu adresa paginii pe care ar trebui sa o mapez am 3 cazuri . Cel in care ma aflu inainte de file size , si pot mapa o pagina de 4096 de bytes direct la file descriptorul fisierului pt output.
Cel in care ma aflu fix pe ultima pagina si trebuie sa mapez pana la file size , iar restul sa le fac 0. Si mai este inca un caz , cel ce imi depaseste spatiul asociat fisierului si intra pe memory size , in cazul acesta mapez o pagina cu anonymous la nici un file descriptor si o setez cu 0 in intregime.