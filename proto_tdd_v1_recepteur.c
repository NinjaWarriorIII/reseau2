/*************************************************************
* proto_tdd_v0 -  récepteur                                  *
* TRANSFERT DE DONNEES  v1                                   *
*                                                            *
* Protocole sans contrôle de flux, sans reprise sur erreurs  *
*                                                            *
* E. Lavinal - Univ. de Toulouse III - Paul Sabatier         *
**************************************************************/

#include <stdio.h>
#include "application.h"
#include "couche_transport.h"
#include "services_reseau.h"

int somme_de_controle(paquet_t paquet);

/* =============================== */
/* Programme principal - récepteur */
/* =============================== */
int main(int argc, char* argv[])
{
    unsigned char message[MAX_INFO]; /* message pour l'application */
    paquet_t paquet; /* paquet utilisé par le protocole */
    int fin = 0; /* condition d'arrêt */

    init_reseau(RECEPTION);

    printf("[TRP] Initialisation reseau : OK.\n");
    printf("[TRP] Debut execution protocole transport.\n");

    /* tant que le récepteur reçoit des données */
    while ( !fin ) {

        // attendre(); /* optionnel ici car de_reseau() fct bloquante */
        de_reseau(&paquet);

        /* extraction des donnees du paquet recu */
        for (int i=0; i<paquet.lg_info; i++) {
            message[i] = paquet.info[i];
        }
        /* remise des données à la couche application */

        if(somme_de_controle(paquet) == 0){
            fin = vers_application(message, paquet.lg_info);
            paquet_t ack;
            ack.type = ACK;
            vers_reseau(&ack);
        }
        else{
            paquet_t ack;
            ack.type = NACK;
            vers_reseau(&ack);
        }

    }

    printf("[TRP] Fin execution protocole transport.\n");
    return 0;
}


int somme_de_controle(paquet_t paquet)
{
    uint8_t control = paquet.type ^ paquet.lg_info ^ paquet.num_seq;
    for(int i = 0; i < paquet.lg_info; i++){
        control ^= paquet.info[i];
    }

    if(control == paquet.somme_ctrl){
        return 0;
    }

    return 1;
}