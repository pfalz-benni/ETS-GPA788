#include "calculateur_li.h"
#include <math.h>

class Calculateur_Leq
{
private:
    uint32_t m_ts;       // Période d'échantillionnage (ms)
    uint16_t m_nbSample; // Calculer le niveau sonore Li toutes les m_ts x m_nbSample ms => 2 secondes
    uint16_t m_nbLi;     // Calculer le niveau de bruit équivalen Leq toutes les m_nbLi x 2 secondes => 5 minutes

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
    inline float GetLeq() const { return m_Leq; }
    inline uint32_t GetTs() const { return m_ts; }
    inline uint16_t GetVrmSamples() const { return m_nbSample; }
    inline uint16_t GetLiSamples() const { return m_nbLi; }

    /* -------------------------------------------------------------
       Services publics offerts
       -------------------------------------------------------------- */

    // Utiliser Accumulate() de l'objet de classe Calculateur_VRMS
    // pour accumuler les valeurs du capteur sonore.
    // Note: La temporisation est reglé dans la fonction (p. 25 - 26). // TODO
    void Accumulate()
    {
        d.Accumulate();
    }

    // Utiliser Compute() de l'objet de classe Calculateur_Li
    // pour calculer la valeur Li et ensuite calculer Leq du signal.
    // Note: La temporisation est reglé dans la fonction (p. 25 - 26). // TODO
    float Compute()
    {
        uint32_t ti = m_ts * m_nbSample;
        uint32_t tp = m_ts * m_nbSample * m_nbLi;
        float sum = 0;

        // TODO Timing: toutes les ti = m_ts x m_nbSample ms
        for (uint32_t i = 0; i < m_nbSample; ++i)
        {
            d.Compute();
            sum += ti * pow(10, 0.1 * d.GetLi());
        }

        m_Leq = 10 * log10(1 / tp * sum);
        return m_Leq;
    }
};