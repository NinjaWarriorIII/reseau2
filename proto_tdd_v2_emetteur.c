/*************************************************************
* proto_tdd_v0 -  émetteur                                   *
* TRANSFERT DE DONNEES  v2                                   *
*                                                            *
* Protocole sans contrôle de flux, sans reprise sur erreurs  *
*                                                            *
* E. Lavinal - Univ. de Toulouse III - Paul Sabatier         *
**************************************************************/

#include <stdio.h>
#include "application.h"
#include "couche_transport.h"
#include "services_reseau.h"

int stop_and_wait_ARQ();

void somme_de_control(paquet_t* paquet);

/* =============================== */
/* Programme principal - émetteur  */
/* =============================== */
int main(int argc, char* argv[])
{
    unsigned char message[MAX_INFO]; /* message de l'application */
    int taille_msg; /* taille du message */
    paquet_t paquet; /* paquet utilisé par le protocole */

    init_reseau(EMISSION);

    printf("[TRP] Initialisation reseau : OK.\n");
    printf("[TRP] Debut execution protocole transport.\n");

    /* lecture de donnees provenant de la couche application */
    de_application(message, &taille_msg);

    /* tant que l'émetteur a des données à envoyer */
    while ( taille_msg != 0 ) {

        /* construction paquet */
        for (int i=0; i<taille_msg; i++) {
            paquet.info[i] = message[i];
        }
        paquet.lg_info = taille_msg;
        paquet.type = DATA;
        somme_de_control(&paquet);

        /* remise à la couche reseau */
        do
        {
            vers_reseau(&paquet);
        } while (stop_and_wait_ARQ());


        /* lecture des donnees suivantes de la couche application */
        de_application(message, &taille_msg);
    }

    printf("[TRP] Fin execution protocole transfert de donnees (TDD).\n");
    return 0;
}

int stop_and_wait_ARQ()
{
    depart_temporisateur(100);
    if (attendre() == -1){
        arret_temporisateur();
        paquet_t paquet;
        de_reseau(&paquet);
        if (paquet.type == ACK){
            printf("aquittement reçu\n");
            return 0;
        }
    }

    return 1;
}

void somme_de_control(paquet_t* paquet)
{
    uint8_t somme_ctrl = paquet->type ^ paquet->lg_info ^ paquet->num_seq;
    for(int i = 0; i < paquet->lg_info; i++){
        somme_ctrl ^= paquet->info[i];
    }

    paquet->somme_ctrl = somme_ctrl;
}