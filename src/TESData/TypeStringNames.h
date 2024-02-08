#ifndef TESDATA_FORMNAMES_H
#define TESDATA_FORMNAMES_H

#include "TESFile/Type.h"

#include <QStringView>

#include <algorithm>
#include <array>
#include <utility>

namespace TESData
{

constexpr auto FormNames = std::to_array<std::pair<TESFile::Type, QStringView>>({
    {"AACT"_ts, u"Actor Action"},
    {"ACHR"_ts, u"Placed NPC"},
    {"ACTI"_ts, u"Activator"},
    {"ADDN"_ts, u"AddOnNode"},
    {"ALCH"_ts, u"Potion"},
    {"AMMO"_ts, u"Ammo"},
    {"ANIO"_ts, u"AnimObject"},
    {"APPA"_ts, u"Alchemical Apparatus"},
    {"ARMA"_ts, u"ArmorAddon"},
    {"ARMO"_ts, u"Armor"},
    {"ARTO"_ts, u"Art Object"},
    {"ASPC"_ts, u"Acoustic Space"},
    {"ASTP"_ts, u"Association Type"},
    {"AVIF"_ts, u"ActorValueInfo"},
    {"BOOK"_ts, u"Book"},
    {"BPTD"_ts, u"BodyPartData"},
    {"CAMS"_ts, u"CameraShot"},
    {"CELL"_ts, u"Cell"},
    {"CLAS"_ts, u"Class"},
    {"CLDC"_ts, u""},
    {"CLFM"_ts, u"ColorForm"},
    {"CLMT"_ts, u"Climate"},
    {"COBJ"_ts, u"Constructible Object"},
    {"COLL"_ts, u"Collision Layer"},
    {"CONT"_ts, u"Container"},
    {"CPTH"_ts, u"Camera Path"},
    {"CSTY"_ts, u"CombatStyle"},
    {"DEBR"_ts, u"Debris"},
    {"DIAL"_ts, u"Dialogue Topic"},
    {"DLBR"_ts, u"Dialogue Branch"},
    {"DLVW"_ts, u"Dialogue View"},
    {"DOBJ"_ts, u"Default Object Manager"},
    {"DOOR"_ts, u"Door"},
    {"DUAL"_ts, u"Dual Cast Data"},
    {"ECZN"_ts, u"Encounter Zone"},
    {"EFSH"_ts, u"EffectShader"},
    {"ENCH"_ts, u"Enchantment"},
    {"EQUP"_ts, u"Equip Slot"},
    {"EXPL"_ts, u"Explosion"},
    {"EYES"_ts, u"Eyes"},
    {"FACT"_ts, u"Faction"},
    {"FLOR"_ts, u"Flora"},
    {"FLST"_ts, u"FormList"},
    {"FSTP"_ts, u"Footstep"},
    {"FSTS"_ts, u"Footstep Set"},
    {"FURN"_ts, u"Furniture"},
    {"GLOB"_ts, u"Global"},
    {"GMST"_ts, u"GameSetting"},
    {"GRAS"_ts, u"Grass"},
    {"HAIR"_ts, u""},
    {"HAZD"_ts, u"Hazard"},
    {"HDPT"_ts, u"HeadPart"},
    {"IDLE"_ts, u"Animation"},
    {"IDLM"_ts, u"IdleMarker"},
    {"IMAD"_ts, u"Imagespace Modifier"},
    {"IMGS"_ts, u"Imagespace"},
    {"INFO"_ts, u"Topic Info"},
    {"INGR"_ts, u"Ingredient"},
    {"IPCT"_ts, u"ImpactData"},
    {"IPDS"_ts, u"ImpactDataSet"},
    {"KEYM"_ts, u"Key"},
    {"KYWD"_ts, u"Keyword"},
    {"LAND"_ts, u"Landscape"},
    {"LCRT"_ts, u"Location Ref Type"},
    {"LCTN"_ts, u"Location"},
    {"LENS"_ts, u"LensFlare"},
    {"LGTM"_ts, u"Lighting Template"},
    {"LIGH"_ts, u"Light"},
    {"LSCR"_ts, u"LoadScreen"},
    {"LTEX"_ts, u"LandTexture"},
    {"LVLI"_ts, u"LeveledItem"},
    {"LVLN"_ts, u"LeveledCharacter"},
    {"LVSP"_ts, u"LeveledSpell"},
    {"MATO"_ts, u"Material Object"},
    {"MATT"_ts, u"Material Type"},
    {"MESG"_ts, u"Message"},
    {"MGEF"_ts, u"Magic Effect"},
    {"MISC"_ts, u"MiscItem"},
    {"MOVT"_ts, u"Movement Type"},
    {"MSTT"_ts, u"MovableStatic"},
    {"MUSC"_ts, u"Music Type"},
    {"MUST"_ts, u"Music Track"},
    {"NAVI"_ts, u"Navigation Mesh Info Map"},
    {"NAVM"_ts, u"Navigation Mesh"},
    {"NPC_"_ts, u"Actor"},
    {"OTFT"_ts, u"Outfit"},
    {"PACK"_ts, u"Package"},
    {"PARW"_ts, u"Placed Arrow"},
    {"PBAR"_ts, u"Placed Barrier"},
    {"PBEA"_ts, u"Placed Beam"},
    {"PCON"_ts, u"Placed Cone/Voice"},
    {"PERK"_ts, u"Perk"},
    {"PFLA"_ts, u"Placed Flame"},
    {"PGRE"_ts, u"Placed Projectile"},
    {"PHZD"_ts, u"Placed Hazard"},
    {"PLYR"_ts, u"Player Reference"},
    {"PMIS"_ts, u"Placed Missile"},
    {"PROJ"_ts, u"Projectile"},
    {"PWAT"_ts, u""},
    {"QUST"_ts, u"Quest"},
    {"RACE"_ts, u"Race"},
    {"REFR"_ts, u"Placed Object"},
    {"REGN"_ts, u"Region"},
    {"RELA"_ts, u"Relationship"},
    {"REVB"_ts, u"Reverb Parameters"},
    {"RFCT"_ts, u"Visual Effect"},
    {"RGDL"_ts, u""},
    {"SCEN"_ts, u"Scene"},
    {"SCOL"_ts, u"Static Collection"},
    {"SCPT"_ts, u""},
    {"SCRL"_ts, u"Scroll"},
    {"SHOU"_ts, u"Shout"},
    {"SLGM"_ts, u"Soul Gem"},
    {"SMBN"_ts, u"Story Manager Branch Node"},
    {"SMEN"_ts, u"Story Manager Event Node"},
    {"SMQN"_ts, u"Story Manager Quest Node"},
    {"SNCT"_ts, u"Sound Category"},
    {"SNDR"_ts, u"Sound Descriptor"},
    {"SOPM"_ts, u"Sound Output Model"},
    {"SOUN"_ts, u"Sound Marker"},
    {"SPEL"_ts, u"Spell"},
    {"SPGD"_ts, u"Shader Particle Geometry"},
    {"STAT"_ts, u"Static"},
    {"TACT"_ts, u"TalkingActivator"},
    {"TES4"_ts, u"File Header"},
    {"TREE"_ts, u"Tree"},
    {"TXST"_ts, u"TextureSet"},
    {"VOLI"_ts, u"VolumetricLighting"},
    {"VTYP"_ts, u"VoiceType"},
    {"WATR"_ts, u"WaterType"},
    {"WEAP"_ts, u"Weapon"},
    {"WOOP"_ts, u"Word of Power"},
    {"WRLD"_ts, u"World Space"},
    {"WTHR"_ts, u"Weather"},
});

template <typename K, typename V, std::size_t N>
inline constexpr V find(const std::array<std::pair<K, V>, N>& table, const K& key)
{
  struct GetKey
  {
    constexpr K operator()(const std::pair<K, V>& element) noexcept
    {
      return element.first;
    }
  };

  const auto it = std::ranges::lower_bound(table, key, std::ranges::less{}, GetKey{});

  if (it != std::ranges::end(table) && it->first == key) {
    return it->second;
  } else {
    return V{};
  }
}

inline constexpr QStringView getFormName(TESFile::Type type)
{
  return find(FormNames, type);
}

}  // namespace TESData

#endif  // TESDATA_FORMNAMES_H
