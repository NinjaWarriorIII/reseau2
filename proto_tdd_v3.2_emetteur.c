/*************************************************************
* proto_tdd_v0 -  émetteur                                   *
* TRANSFERT DE DONNEES  v2                                   *
*                                                            *
* Protocole sans contrôle de flux, sans reprise sur erreurs  *
*                                                            *
* E. Lavinal - Univ. de Toulouse III - Paul Sabatier         *
**************************************************************/

#define taille_fenetre 4
#define numerotation 16

#include <stdio.h>
#include "application.h"
#include "couche_transport.h"
#include "services_reseau.h"


int go_back_n(uint8_t* numero_paquet, paquet_t fenetre[]);

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

    uint8_t numero_paquet = 0;
    paquet_t fenetre[4];
    for(int i = 0; i < 4; ++i){
        fenetre[i].type = OTHER;
    }

    printf("[TRP] Initialisation reseau : OK.\n");
    printf("[TRP] Debut execution protocole transport.\n");

    /* lecture de donnees provenant de la couche application */
    de_application(message, &taille_msg);

    /* tant que l'émetteur a des données à envoyer */
    while ( taille_msg != 0 || fenetre[0].type != OTHER) {
            for(int i = 0; i < 4; ++i){
                /* construction paquet */
                if (fenetre[i].type == OTHER && taille_msg!=0){
                    for (int i=0; i<taille_msg; i++) {
                        paquet.info[i] = message[i];
                    }
                    paquet.num_seq = (numero_paquet+i)%numerotation;
                    paquet.lg_info = taille_msg;
                    paquet.type = DATA;
                    somme_de_control(&paquet);

                    vers_reseau(&paquet);

                    fenetre[i] = paquet;
                    /* lecture des donnees suivantes de la couche application */
                    de_application(message, &taille_msg);
                }
                else{
                    vers_reseau(&fenetre[i]);
                }
            }
        
        go_back_n(&numero_paquet, fenetre);

    }

    printf("[TRP] Fin execution protocole transfert de donnees (TDD).\n");
    return 0;
}

int go_back_n(uint8_t* numero_paquet, paquet_t fenetre[])
{
    depart_temporisateur(100);
    while(attendre() == -1){
        paquet_t ack;
        de_reseau(&ack);
        printf("le numero packet et le ack : %d, %d", *numero_paquet, ack.num_seq);
        if(*numero_paquet == ack.num_seq){
            *numero_paquet = (*numero_paquet+1)%numerotation;
            for(int i =0; i < 3; ++i){
                fenetre[i] = fenetre[i+1];
            }
            fenetre[3].type = OTHER;
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