#include "calculateur_li.h"

class Calculateur_Leq
{
private:
    uint32_t m_ts;
    uint16_t m_nbSample;
    uint16_t m_nbLi;

    float m_Leq;
    Calculateur_Li d; // Initialized with standard parameters

public:
    Calculateur_Leq(const uint32_t ts, const uint16_t nbSample, const uint16_t nbLi) // TODO which parameters
        : m_ts(ts), m_nbSample(nbSample), m_nbLi(nbLi)
    {
    }

    // Empêcher l'utilisation du constructeur de copie
    Calculateur_Leq(const Calculateur_Leq &other) = delete;
    // Empêcher l'utilisation de l'opérateur d'assignation
    Calculateur_Leq &operator=(const Calculateur_Leq &other) = delete;
    // Empêcher l'utilisation du constructeur par déplacement
    Calculateur_Leq(Calculateur_Leq &&other) = delete;
    // Empêcher l'utilisation de l'opérateur de déplacement
    Calculateur_Leq &operator=(Calculateur_Leq &&other) = delete;

    ~Calculateur_Leq() = default;

    /* -------------------------------------------------------------
       Accesseurs des données membres de la classe
       -------------------------------------------------------------- */
    inline double GetLeq() const { return m_Leq; }

    /* -------------------------------------------------------------
       Services publics offerts
       -------------------------------------------------------------- */
    // Utiliser Accumulate() de l'objet de classe Calculateur_VRMS
    // pour accumuler les valeurs du capteur sonore.
    // Note: La temporisation est la responsabilité de l'utilisateur.
    void Accumulate()
    {
        d.Accumulate();
    }
    // Utiliser Compute() de l'objet de classe Calculateur_VRMS
    // pour calculer la valeur rms du signal sonore et ensuite
    // calculer Li du signal.
    // Note: La temporisation est la responsabilité de l'utilisateur.
    double Compute()
    {
        d.Compute();
        // m_Leg = // TODO
        return m_Leq;
    }
};