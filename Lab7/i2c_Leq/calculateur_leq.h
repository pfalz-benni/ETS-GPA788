/*
 * Calculateur_Li
 *
 * Une classe pour calculer Leq, le niveau d'énergie sonore équivalent
 * pour une durée déterminé, à l'aide des classes Calculateur_Li et
 * Calculateur_VRMS.
 *
 * Auteurs : Philippe Boivin, Sandrine Bouchard, Alexandre Lins-d'Auteuil,
 * Benedikt Franz Witteler
 *
 * Dans le cadre du cours :
 * GPA788 - ETS
 * T. Wong
 * 10-2019
 * 07-2020
 */

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
    Calculateur_Leq(const uint32_t ts, const uint16_t nbSample, const uint16_t nbLi)
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

    /**
     * @brief Utiliser Accumulate() de l'objet de classe Calculateur_VRMS
     * pour accumuler les valeurs du capteur sonore.
     * Note: La temporisation est reglé dans la fonction (p. 25 - 26).
     */
    void Accumulate()
    {
        static unsigned long start = millis();
        if (millis() - start >= m_ts)
        {
            d.Accumulate();
            start = millis();
        }
    }

    /**
     * @brief Utiliser Compute() de l'objet de classe Calculateur_Li
     * pour calculer la valeur Li et ensuite calculer Leq du signal.
     * Note: La temporisation est reglé dans la fonction (p. 25 - 26).
     */
    bool Compute()
    {
        static uint32_t nb_li_count = 0;
        static float sum = 0;

        // Timing: toutes les ti = m_ts x m_nbSample ms
        if (d.GetNbSamples() == m_nbSample)
        {
            d.Compute();
            float ti = m_ts * m_nbSample;
            sum += ti * pow(10.0, 0.1 * d.GetLi());
            ++nb_li_count;
        }

        if (nb_li_count == m_nbLi)
        {
            float tp = m_ts * m_nbSample * m_nbLi;

            m_Leq = 10.0 * log10(1.0 / tp * sum);
            nb_li_count = 0;
            sum = 0;
            return true;
        }
        return false;
    }
};