/*
 * Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2006-2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "blackrock_spire.h"

enum Text
{
    EMOTE_ONE_STACK                 = 0,
    EMOTE_TEN_STACK                 = 1,
    EMOTE_FREE_OF_BONDS             = 2,
    YELL_FREE_OF_BONDS              = 3
};

enum Spells
{
    SPELL_ENCAGED_EMBERSEER         = 15282, // Self on spawn
    SPELL_FIRE_SHIELD_TRIGGER       = 13377, // Self on spawn missing from 335 dbc
    SPELL_FREEZE_ANIM               = 16245, // Self on event start
    SPELL_EMBERSEER_GROWING         = 16048, // Self on event start
    SPELL_EMBERSEER_FULL_STRENGTH   = 16047, // Emberseer Full Strength
    SPELL_FIRENOVA                  = 23462, // Combat
    SPELL_FLAMEBUFFET               = 23341, // Combat
    SPELL_PYROBLAST                 = 17274  // Combat
};

enum Events
{
    // OOC
    EVENT_RESPAWN                   = 1,
    // Combat
    EVENT_FIRENOVA                  = 2,
    EVENT_FLAMEBUFFET               = 3,
    EVENT_PYROBLAST                 = 4
};

class boss_pyroguard_emberseer : public CreatureScript
{
public:
    boss_pyroguard_emberseer() : CreatureScript("boss_pyroguard_emberseer") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_pyroguard_emberseerAI(creature);
    }

    struct boss_pyroguard_emberseerAI : public BossAI
    {
        boss_pyroguard_emberseerAI(Creature* creature) : BossAI(creature, DATA_PYROGAURD_EMBERSEER) {}

        void Reset()
        {
            if (instance)
            {
                // Apply auras on spawn and reset
                DoCast(me, SPELL_ENCAGED_EMBERSEER);

                // DoCast(me, SPELL_FIRE_SHIELD_TRIGGER); // Need to find this in old DBC if possible

                // Open doors on reset
                if (instance->GetBossState(DATA_PYROGAURD_EMBERSEER) == IN_PROGRESS)
                    OpenDoors(false); // Opens 2 entrance doors

                // ### TODO Reset Blackrock Altar ###

                // respawn any dead Blackhand Incarcerators
                events.ScheduleEvent(EVENT_RESPAWN, 1000);


                instance->SetBossState(DATA_PYROGAURD_EMBERSEER, NOT_STARTED);
            }
        }

        void SetData(uint32 type, uint32 data)
        {
            if (instance && type == 1 && data == 1)
            {
                instance->SetBossState(DATA_PYROGAURD_EMBERSEER, IN_PROGRESS);

               // Close these two doors on encounter start
               if (GameObject* door1 = me->GetMap()->GetGameObject(instance->GetData64(GO_EMBERSEER_IN)))
                   door1->SetGoState(GO_STATE_READY);
               if (GameObject* door2 = me->GetMap()->GetGameObject(instance->GetData64(GO_DOORS)))
                   door2->SetGoState(GO_STATE_READY);

                // ### TODO Script pre fight scripting of Emberseer ###
                // ### TODO Set data on Blackhand Incarcerator ###
                // ### TODO disable Blackrock Altar ###
            }


        }

        void EnterCombat(Unit* /*who*/)
        {
            // ### TODO Check combat timing ###
            events.ScheduleEvent(EVENT_FIRENOVA,    6000);
            events.ScheduleEvent(EVENT_FLAMEBUFFET, 3000);
            events.ScheduleEvent(EVENT_PYROBLAST,  14000);
        }

        void JustDied(Unit* /*killer*/)
        {

            if (instance)
            {
                // Activate all the runes
                if (GameObject* rune1 = me->GetMap()->GetGameObject(instance->GetData64(GO_EMBERSEER_RUNE_1)))
                    rune1->SetGoState(GO_STATE_READY);
                if (GameObject* rune2 = me->GetMap()->GetGameObject(instance->GetData64(GO_EMBERSEER_RUNE_2)))
                    rune2->SetGoState(GO_STATE_READY);
                if (GameObject* rune3 = me->GetMap()->GetGameObject(instance->GetData64(GO_EMBERSEER_RUNE_3)))
                    rune3->SetGoState(GO_STATE_READY);
                if (GameObject* rune4 = me->GetMap()->GetGameObject(instance->GetData64(GO_EMBERSEER_RUNE_4)))
                    rune4->SetGoState(GO_STATE_READY);
                if (GameObject* rune5 = me->GetMap()->GetGameObject(instance->GetData64(GO_EMBERSEER_RUNE_5)))
                    rune5->SetGoState(GO_STATE_READY);
                if (GameObject* rune6 = me->GetMap()->GetGameObject(instance->GetData64(GO_EMBERSEER_RUNE_6)))
                    rune6->SetGoState(GO_STATE_READY);
                if (GameObject* rune7 = me->GetMap()->GetGameObject(instance->GetData64(GO_EMBERSEER_RUNE_7)))
                    rune7->SetGoState(GO_STATE_READY);

                // Opens all 3 doors
                OpenDoors(true);

                instance->SetBossState(DATA_PYROGAURD_EMBERSEER, DONE);
            }
        }

       void OpenDoors(bool Boss_Killed)
       {
           // These two doors reopen on reset or boss kill
           if (GameObject* door1 = me->GetMap()->GetGameObject(instance->GetData64(GO_EMBERSEER_IN)))
               door1->SetGoState(GO_STATE_ACTIVE);
           if (GameObject* door2 = me->GetMap()->GetGameObject(instance->GetData64(GO_DOORS)))
               door2->SetGoState(GO_STATE_ACTIVE);

           // This door opens on boss kill
           if (Boss_Killed)
               if (GameObject* door3 = me->GetMap()->GetGameObject(instance->GetData64(GO_EMBERSEER_OUT)))
                    door3->SetGoState(GO_STATE_ACTIVE);
       }

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            if (!UpdateVictim())
            {
                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_RESPAWN:
                            std::list<Creature*> creatureList;
                            GetCreatureListWithEntryInGrid(creatureList, me, NPC_BLACKHAND_INCARCERATOR, 35.0f);
                            for (std::list<Creature*>::iterator itr = creatureList.begin(); itr != creatureList.end(); ++itr)
                                if (Creature* creatureList = *itr)
                                    if (!creatureList->IsAlive())
                                        creatureList->Respawn();
                            break;
                    }
                }
                return;
            }

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_FIRENOVA:
                        DoCastVictim(SPELL_FIRENOVA);
                        events.ScheduleEvent(EVENT_FIRENOVA, 6000);
                        break;
                    case EVENT_FLAMEBUFFET:
                        DoCastVictim(SPELL_FLAMEBUFFET);
                        events.ScheduleEvent(EVENT_FLAMEBUFFET, 14000);
                        break;
                    case EVENT_PYROBLAST:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true))
                            DoCast(target, SPELL_PYROBLAST);
                        events.ScheduleEvent(EVENT_PYROBLAST, 15000);
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };
};

/*####
## npc_blackhand_incarcerator
####*/

enum IncarceratorSpells
{
    SPELL_ENCAGE_EMBERSEER          = 15281, // Emberseer on spawn
    SPELL_STRIKE                    = 15580, // Combat
    SPELL_ENCAGE                    = 16045  // Combat
};

enum IncarceratorEvents
{
    // Out of combat
    EVENT_ENCAGED_EMBERSEER         = 1,
    // Combat
    EVENT_STRIKE                    = 2,
    EVENT_ENCAGE                    = 3
};

class npc_blackhand_incarcerator : public CreatureScript
{
public:
    npc_blackhand_incarcerator() : CreatureScript("npc_blackhand_incarcerator") { }

    struct npc_blackhand_incarceratorAI : public ScriptedAI
    {
        npc_blackhand_incarceratorAI(Creature* creature) : ScriptedAI(creature) {}

        void Reset()
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC|UNIT_FLAG_IMMUNE_TO_NPC);

            _events.ScheduleEvent(EVENT_ENCAGED_EMBERSEER, 1000);
        }

        void JustDied(Unit* /*killer*/)
        {
            me->DespawnOrUnsummon(10000);
        }

        void SetData(uint32 data, uint32 value)
        {
            if (data == 1 && value == 1)
            {
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC|UNIT_FLAG_IMMUNE_TO_NPC);
                me->RemoveAura(SPELL_ENCAGE_EMBERSEER);
                // ### TODO attack nearest target ###
            }

        }
        void EnterCombat(Unit* /*who*/)
        {
            _events.ScheduleEvent(EVENT_STRIKE, urand(8000, 16000));
            _events.ScheduleEvent(EVENT_ENCAGE, urand(10000, 20000));
        }

        void UpdateAI(uint32 diff)
        {
            _events.Update(diff);

            if (!UpdateVictim())
            {
                while (uint32 eventId = _events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_ENCAGED_EMBERSEER:
                            if(!me->HasAura(SPELL_ENCAGE_EMBERSEER))
                                if (Creature* Emberseer = me->FindNearestCreature(NPC_PYROGAURD_EMBERSEER, 30.0f, true))
                                    DoCast(Emberseer, SPELL_ENCAGE_EMBERSEER);
                            _events.ScheduleEvent(EVENT_ENCAGED_EMBERSEER, 1000);
                            break;
                    }
                }
                return;
            }

            while (uint32 eventId = _events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_STRIKE:
                        DoCastVictim(SPELL_STRIKE, true);
                        _events.ScheduleEvent(EVENT_STRIKE, urand(14000, 23000));
                        break;
                    case EVENT_ENCAGE:
                        DoCast(SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true), EVENT_ENCAGE, true);
                        _events.ScheduleEvent(EVENT_ENCAGE, urand(6000, 12000));
                        break;
                    default:
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }

        private:
            EventMap _events;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_blackhand_incarceratorAI(creature);
    }
};

void AddSC_boss_pyroguard_emberseer()
{
    new boss_pyroguard_emberseer();
    new npc_blackhand_incarcerator();
}
