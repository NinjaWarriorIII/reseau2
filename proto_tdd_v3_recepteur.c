/*************************************************************
* proto_tdd_v0 -  récepteur                                  *
* TRANSFERT DE DONNEES  v2                                   *
*                                                            *
* Protocole sans contrôle de flux, sans reprise sur erreurs  *
*                                                            *
* E. Lavinal - Univ. de Toulouse III - Paul Sabatier         *
**************************************************************/

#define numerotation 16

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

    uint8_t numero_paquet = 0;

    init_reseau(RECEPTION);

    printf("[TRP] Initialisation reseau : OK.\n");
    printf("[TRP] Debut execution protocole transport.\n");

    /* tant que le récepteur reçoit des données */
    while ( !fin ) {

        de_reseau(&paquet);

        if(somme_de_controle(paquet)==0){
            if(paquet.num_seq == numero_paquet){
                paquet_t ack;
                ack.num_seq = numero_paquet;
                ack.type = ACK;
                vers_reseau(&ack);

                for (int i=0; i<paquet.lg_info; i++) {
                    message[i] = paquet.info[i];
                }
                fin = vers_application(message, paquet.lg_info);
                numero_paquet = (numero_paquet+1)%numerotation;

            }
            else{
                paquet_t ack;
                ack.num_seq = (numero_paquet-1)%numerotation;
                ack.type = ACK;
                de_reseau(&ack);
            }
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