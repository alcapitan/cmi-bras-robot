#include <Pixy2.h>
#include <Pixy2CCC.h>
#include <SoftwareSerial.h>
using namespace std;

int dureeMouvementMoteur = 2000;
SoftwareSerial mySerial(4, 5); // RX (réception), TX (transmission)
SoftwareSerial BT(8, 9); // RX (réception), TX (transmission)
Pixy2 pixy;
// Serial = interface de test
// mySerial = interface physique
// BT = interface mobile
// pixy = caméra

/* correspondance couleurs :
 *  1 = rouge
 *  2 = bleu
 *  3 = jaune
 *  4 = vert
 */
int couleur_cherche = 2;

struct Objet {
    uint16_t x, y, width, height, color;
};

void definirDebitTransmission(int rate)
{
    // met une fréquence commune à tous les appareils
    // nous avons utilisé 9600 MHz
    Serial.begin(rate);
    mySerial.begin(rate);
    BT.begin(rate);
}

bool verifierAngleMoteur(int motorId, int angle)
{
    // vérifie si l'angle voulu est inclus dans l'intervalle que l'on a défini
    switch (motorId) {
    case 0:
        return (angle >= 500 && angle <= 2500);
    case 1:
        return (angle >= 1000 && angle <= 2000);
    case 2:
        return (angle >= 1000 && angle <= 2000);
    case 3:
        return (angle >= 500 && angle <= 1500);
    case 4:
        return (angle >= 1300 && angle <= 2250);
    case 5:
        return (angle >= 1000 && angle <= 1800);
    default:
        return false;
    }
}

void ordreMoteur(int motorId, int angle)
{
    // exécute le mouvement moteur
    if (!verifierAngleMoteur(motorId, angle)) {
        String message = "ERREUR : Angle invalide pour le moteur #" + String(motorId) + " : " + String(angle) + ".";
        Serial.println(message);
        return;
    }
    String commande = "#" + String(motorId) + "P" + String(angle) + "T" + String(dureeMouvementMoteur);
    // Serial.println("Mouvement moteur : " + commande);
    mySerial.println(commande);
}

void positionRepos()
{
    /* combinaison de mouvements pour mettre le bras en position de repos */
    ordreMoteur(0, 1500);
    ordreMoteur(1, 2000);
    ordreMoteur(2, 1800);
    ordreMoteur(3, 500);
    ordreMoteur(4, 2250);
    ordreMoteur(5, 1000);
}

Objet convertirDonneesObjet(Block block)
{
    // converti l'objet Pixy en notre propre objet plus lisible
    // ceci est un objet détecté par la caméra
    Objet coor;
    coor.x = block.m_x;
    coor.y = block.m_y;
    coor.width = block.m_width;
    coor.height = block.m_height;
    coor.color = block.m_signature;
    return coor;
}

void setup()
{
    // définir le débit de transmission de données pour établir la communication
    definirDebitTransmission(9600);

    pixy.init();

    // mise en position de repos quand l'initialisation est faite
    positionRepos();
    delay(dureeMouvementMoteur + 500); // attendre que les mouvements précédents se terminent + 500 millisecondes
}

void loop()
{
    // cette fonction s'exécute en tant que boucle infinie après l'initialisation

    // récupérer les éléments détectés par la caméra
    pixy.ccc.getBlocks();

    bool objet_prendre = false;
    Objet element;

    if (!BT.available()) {
        Serial.println("Le module Bluetooth ne communique pas.");
    }

    if (pixy.ccc.numBlocks < 1) {
        Serial.println("Aucun bloc détecté, mise en position de repos.");
        positionRepos();
    } else {
        // couleur_cherche = BT.read(); // commande pour récupérer l'identifiant du bouton cliqué, et donc l'action à faire depuis l'action utilisateur
        for (int i = 0; i < pixy.ccc.numBlocks; i++) {
            if (pixy.ccc.blocks[i].m_signature == couleur_cherche) { // récupère un bloc détecté qui correspond à ce que l'on cherche
                objet_prendre = true;
                element = convertirDonneesObjet(pixy.ccc.blocks[i]);
                break;
            }
        }

        if (objet_prendre == false) { // aucun bloc détecté ne correspond à ce que l'on recherche
            Serial.println("Aucun bloc de la couleur recherchée détecté, mise en position de repos.");
            positionRepos();
        } else {
            Serial.println("Bloc détecté");
            /*Serial.print("x = ");
            Serial.println(element.x);
            Serial.print("y = ");
            Serial.println(element.y);
            Serial.print("width = ");
            Serial.println(element.width);
            Serial.print("height = ");
            Serial.println(element.height);*/
            // on calcule la cible où le bras robot devrait pointer
            int cible_x = (element.x / 2) + element.x;
            int cible_y = (element.y / 2) + element.y;
            Serial.print("x ciblé = ");
            Serial.println(cible_x);
            Serial.print("y ciblé = ");
            Serial.println(cible_y);
            Serial.print("color = ");
            if (element.color == 1)
                Serial.println("rouge");
            else if (element.color == 2)
                Serial.println("bleu");
            else if (element.color == 3)
                Serial.println("jaune");
            else if (element.color == 4)
                Serial.println("vert");
            else
                Serial.println("autre");
        }
    }
    // on n'a pas pu coordonner la détection et les mouvements moteurs, mais on sait au moins interpréter les résultats
    delay(dureeMouvementMoteur + 500);
}
