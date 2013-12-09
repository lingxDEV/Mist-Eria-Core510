/*
* Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
* Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
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

#include "Common.h"
#include "DBCEnums.h"
#include "ObjectMgr.h"
#include "ArenaTeamMgr.h"
#include "GuildMgr.h"
#include "World.h"
#include "WorldPacket.h"
#include "DatabaseEnv.h"
#include "AchievementMgr.h"
#include "ArenaTeam.h"
#include "CellImpl.h"
#include "GameEventMgr.h"
#include "GridNotifiersImpl.h"
#include "Guild.h"
#include "Language.h"
#include "Player.h"
#include "SpellMgr.h"
#include "DisableMgr.h"
#include "ScriptMgr.h"
#include "MapManager.h"
#include "Battleground.h"
#include "BattlegroundAB.h"
#include "Map.h"
#include "InstanceScript.h"
#include "Group.h"
#include "BattlePetMgr.h"

BattlePetMgr::BattlePetMgr(Player* owner) : m_player(owner)
{
}

void BattlePetMgr::GetBattlePetList(PetBattleDataList &battlePetList) const
{
    PlayerSpellMap spellMap = m_player->GetSpellMap();

    for (PlayerSpellMap::iterator itr = spellMap.begin(); itr != spellMap.end(); ++itr) {
        PlayerSpell* s = itr->second;


        if (s->state == PLAYERSPELL_REMOVED)
            continue;

        if (!s->active || s->disabled)
            continue;

        SpellInfo const* spell = sSpellMgr->GetSpellInfo(itr->first);
        if (!spell)
            continue;

        // Is summon pet spell
        if ((spell->Effects[0].Effect == SPELL_EFFECT_SUMMON && spell->Effects[0].MiscValueB == 3221) == 0)
            continue;

        const CreatureTemplate* creature = sObjectMgr->GetCreatureTemplate(spell->Effects[0].MiscValue);
        if (!creature)
            continue;

        const BattlePetSpeciesEntry* species = sBattlePetSpeciesStore.LookupEntry(creature->Entry);
        if (!species)
            continue;

        PetBattleData pet(creature->Entry, creature->Modelid1, species->ID, spell->Id);
        battlePetList.push_back(pet);
    }
}

void BattlePetMgr::BuildBattlePetJournal(WorldPacket *data)
{
    PetBattleDataList petList;
    GetBattlePetList(petList);

    uint8 gap81_8, gap81_7 = 0;

    uint16 dword18, dword14, dword16, dword1A = 1;
    std::string name = "toto";

    uint32 dword20, dword24, dword1C, dword28, dwordC = 1;
    ObjectGuid guid, guid2 = 0;

    data->Initialize(SMSG_BATTLE_PET_JOURNAL);
    *data << uint16(0); // unk
    data->WriteBit(0); // unk

    data->WriteBits(petList.size(), 20);

    // bits part
    for (PetBattleDataList::iterator pet = petList.begin(); pet != petList.end(); ++pet) {
        data->WriteBit(!gap81_8);
        data->WriteBit(guid[0]);
        data->WriteBit(guid[7]);
        data->WriteBit(guid2!=0);
        if (guid2) {
            data->WriteBit(guid2[0]);
            data->WriteBit(guid2[5]);
            data->WriteBit(guid2[2]);
            data->WriteBit(guid2[1]);
            data->WriteBit(guid2[7]);
            data->WriteBit(guid2[3]);
            data->WriteBit(guid2[4]);
            data->WriteBit(guid2[6]);
        }
        data->WriteBit(guid[6]);
        data->WriteBit(guid[2]);
        data->WriteBit(gap81_7);
        data->WriteBit(!dword14);
        data->WriteBit(guid[4]);
        data->WriteBit(guid[3]);
        data->WriteBit(guid[5]);
        data->WriteBit(!dword18);
        data->WriteBit(guid[1]);

        data->WriteBits(name.size(), 7); // name lenght
    }

    data->WriteBits(0, 21); // unk counter, may be related to battle pet slot

    // data part
    for (PetBattleDataList::iterator pet = petList.begin(); pet != petList.end(); ++pet) {
        data->WriteByteSeq(guid[3]);
        *data << uint32(dword24);                       // UNK
        *data << uint32(dwordC);                        // UNK
        if (gap81_8) *data << uint8(gap81_8);           // UNK
        data->WriteByteSeq(guid[5]);
        *data << uint32(dword28);                       // UNK
        if (dword18) *data << uint16(dword18);          // UNK

        if (guid2) {
            data->WriteByteSeq(guid2[0]);
            data->WriteByteSeq(guid2[3]);
            data->WriteByteSeq(guid2[2]);
            data->WriteByteSeq(guid2[5]);
            data->WriteByteSeq(guid2[4]);
            *data << uint32(0);
            data->WriteByteSeq(guid2[6]);
            data->WriteByteSeq(guid2[7]);
            data->WriteByteSeq(guid2[1]);
        }

        data->WriteByteSeq(guid[6]);
        *data << uint16(dword1A);                             // UNK
        data->WriteByteSeq(guid[7]);
        data->WriteByteSeq(guid[1]);
        *data << uint32(dword1C);                       // UNK
        if (dword14) *data << uint16(dword14);          // UNK
        data->WriteByteSeq(guid[4]);
        data->WriteByteSeq(guid[2]);
        data->WriteString(name);
        data->WriteByteSeq(guid[0]);
        *data << uint32((*pet).m_speciesID);
        *data << uint32((*pet).m_displayID);
        *data << uint32(dword20);                       // UNK
        *data << uint16(dword16);                       // UNK
    }

/*
    *data << uint16(0); // unk
    data->WriteBit(1); // unk
    data->WriteBits(0, 20); // unk counter, may be related to battle pet slot

    GetBattlePetList(petList);
    data->WriteBits(petList.size(), 19);

    // bits part
    for (auto pet : petList)
    {
        data->WriteBit(true); // hasBreed, inverse
        data->WriteBit(true); // hasQuality, inverse
        data->WriteBit(true); // hasUnk, inverse
        data->WriteBits(0, 7); // name lenght
        data->WriteBit(false); // unk bit
        data->WriteBit(false); // has guid
    }

    // data part
    for (auto pet : petList)
    {
        *data << uint32(pet.m_displayID);
        *data << uint32(pet.m_summonSpellID); // Pet Entry
        *data << uint16(0); // xp
        *data << uint32(1); // health
        *data << uint16(1); // level
        // name
        *data << uint32(1); // speed
        *data << uint32(1); // max health
        *data << uint32(pet.m_entry); // Creature ID
        *data << uint32(1); // power
        *data << uint32(pet.m_speciesID); // species
    }

    */

}

void WorldSession::HandleSummonBattlePet(WorldPacket& recvData)
{
    uint32 spellID = 0;
    recvData >> spellID;

    if (!_player->HasSpell(spellID))
        return;

    _player->CastSpell(_player, spellID, true);
}
