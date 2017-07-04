#ifndef _MTWISTER_H_5cf039de_248a_4c8d_b583_90698d300c3e_
#define _MTWISTER_H_5cf039de_248a_4c8d_b583_90698d300c3e_

#include <stdint.h>

class CMTwister
{
protected:
    uint32_t m_Twist[624];
    uint32_t m_Count;

public:
    CMTwister()
    {
        m_Count = 625;
    }

    CMTwister(uint32_t Seed)
    {
        SetSeed(Seed);
    }

    void SetSeed(uint32_t Seed)
    {
        uint32_t  Temp;
        uint32_t *pTwist;

        pTwist = m_Twist;
        for (uint32_t i = ARRAYSIZE(m_Twist); i; --i)
        {
            Temp = Seed;
            Seed = Seed * 0x10DCD + 1;
            *pTwist++ = (Temp & 0xFFFF0000) | (Seed >> 16);
            Seed = Seed * 0x10DCD + 1;
        }

        m_Count = ARRAYSIZE(m_Twist);
    }

    VOID SetSeed2(ULONG Seed)
    {
        ULONG   Temp;
        PULONG  Twist;

        Twist = (PULONG)m_Twist;
        *Twist++ = Seed;
        for (ULONG_PTR Index = 1, Count = ARRAYSIZE(m_Twist) - 1; Count; ++Index, --Count)
        {
            Temp = Twist[-1];
            *Twist++ = (Temp ^ (Temp >> 30)) * 0x6C078965 + Index;
        }

        m_Count = ARRAYSIZE(m_Twist);
    }

    uint32_t GetRandom()
    {
        uint32_t  Index, val;
        uint32_t *Twist;

#define GET_MASK(val) (((val) & 1) ? 0x9908B0DF : 0)

        if (m_Count >= ARRAYSIZE(m_Twist))
        {
            Twist = m_Twist;

            for (Index = 0; Index != 0xE3; ++Twist, ++Index)
            {
                val = Twist[1];
                val = (val ^ Twist[0]) & 0x7FFFFFFF;
                val ^= Twist[0];
                val = (val >> 1) ^ GET_MASK(val);
                val ^= Twist[396 + 1];
                Twist[0] = val;
            }

            while (Index < ARRAYSIZE(m_Twist) - 1)
            {
                val = (Twist[0] ^ Twist[1]) & 0x7FFFFFFF;
                val ^= Twist[0];
                val = (val >> 1) ^ (GET_MASK(val) ^ Twist[-0xE4 + 1]);
                Twist[0] = val;

                ++Twist;
                ++Index;
            }

            val = m_Twist[0] ^ m_Twist[ARRAYSIZE(m_Twist) - 1];
            val &= 0x7FFFFFFF;
            val ^= m_Twist[ARRAYSIZE(m_Twist) - 1];
            val = (val >> 1) ^ GET_MASK(val);
            m_Twist[ARRAYSIZE(m_Twist) - 1] = val ^ m_Twist[396];
            Index = 0;
        }
        else
        {
            Index = m_Count;
        }

#undef GET_MASK

        val = m_Twist[Index++];
        m_Count = Index;
        val ^= val >> 0x0B;
        val ^= (val & 0xFF3A58AD) << 0x07;
        val ^= (val & 0xFFFFDF8C) << 0x0F;
        val ^= val >> 0x12;

        return val;
    }
};

#endif // _MTWISTER_H_5cf039de_248a_4c8d_b583_90698d300c3e_
