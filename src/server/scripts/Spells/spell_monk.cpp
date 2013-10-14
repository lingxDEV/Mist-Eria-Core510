#include "Player.h"
#include "ScriptMgr.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "MapManager.h"

// 116095 - Disable
class spell_monk_disable : public SpellScriptLoader
{
public:
    spell_monk_disable() : SpellScriptLoader("spell_monk_disable") { }

    class spell_monk_disable_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_monk_disable_AuraScript);

        void PeriodicDummy(AuraEffect const* aurEff)
        {
            if(GetUnitOwner() && GetCaster())
            {
                if(GetUnitOwner()->IsWithinDist(GetCaster(), 10.f, true))
                    aurEff->GetBase()->RefreshDuration();
            }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_monk_disable_AuraScript::PeriodicDummy, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_monk_disable_AuraScript();
    }
    
    class spell_monk_disable_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_monk_disable_SpellScript);

        void Cast()
        {
            if (Unit* target = GetExplTargetUnit())
            {
                //Cant apply twice
                if(target->HasAura(116706))
                {
                    PreventHitAura();
                    return;
                }

                if(target->HasAura(116095))
                {
                    AuraEffect* tickEffect = target->GetAuraEffect(116095, EFFECT_1, GetCaster()->GetGUID());
                    if(tickEffect && tickEffect->GetTickNumber() > 1)
                    {
                        GetCaster()->CastSpell(target, 116706, true);
                        target->RemoveAura(116095);
                    }
                    PreventHitAura();
                }
                
            }
        }

        void Register()
        {
            OnCast += SpellCastFn(spell_monk_disable_SpellScript::Cast);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_monk_disable_SpellScript();
    }
    
};

class spell_monk_paralysis : public SpellScriptLoader
{
public:
    spell_monk_paralysis() : SpellScriptLoader("spell_monk_paralysis") { }


    class spell_monk_paralysis_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_monk_paralysis_AuraScript);

        void OnAmountCalc(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
        {
            if(GetCaster() && GetTarget() && GetCaster()->isInBack(GetTarget()))
            {
                SetMaxDuration(GetMaxDuration()*2);
            }
        }

        void Register()
        {
            OnEffectApply += AuraEffectApplyFn(spell_monk_paralysis_AuraScript::OnAmountCalc, EFFECT_0, SPELL_AURA_MOD_STUN, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript* GetAuraScript() const
    {
        return new spell_monk_paralysis_AuraScript();
    }
};

// 119996 - transcandence_transfert
class spell_monk_transcandence_transfert : public SpellScriptLoader
{
public:
    spell_monk_transcandence_transfert() : SpellScriptLoader("spell_monk_transcandence_transfert") { }

    class spell_monk_transcandence_transfert_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_monk_transcandence_transfert_SpellScript);

        void Hit()
        {
            if(Creature* target = GetHitCreature())
            {
                Position *petPos, *casterPos;
                uint32 petMapId, casterMapId;
                target->GetPosition(petPos);
                petMapId = target->GetMapId();
                GetCaster()->GetPosition(casterPos);
                casterMapId = GetCaster()->GetMapId();

                //target->FarTeleportTo(GetCaster()->GetMap(), casterPos->GetPositionX(), casterPos->GetPositionY(), casterPos->GetPositionZ(), casterPos->GetOrientation());

                //if(GetCaster()->ToPlayer())
                    //GetCaster()->ToPlayer()->TeleportTo(petMapId, petPos->GetPositionX(), petPos->GetPositionY(), petPos->GetPositionZ(), petPos->GetOrientation());

            }
        }

        void Register()
        {
            OnHit += SpellHitFn(spell_monk_transcandence_transfert_SpellScript::Hit);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_monk_transcandence_transfert_SpellScript();
    }
};


void AddSC_monk_spell_scripts()
{
    new spell_monk_disable();
    new spell_monk_paralysis();
    new spell_monk_transcandence_transfert();
}