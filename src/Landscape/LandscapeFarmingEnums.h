//
// LandscapeFarmingEnums.h
//
/*

Copyright (c) 2019, National Environmental Research Institute, Denmark (NERI)

All rights reserved.


Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

*) Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimer. *) Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution. *) Neither the name of the NERI nor the names of its contributors
may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/


#ifndef LANDSCAPEANDFARMINGENUMS
#define LANDSCAPEANDFARMINGENUMS

//********************************************************************************************************
//****************************** Landscape Enums *********************************************************
//********************************************************************************************************

/**********************************************************************************************//**
 * Enum:	TTypeOfDirections
 *
 * Brief:	/**  \brief ALMaSS 8 standard directions
 **************************************************************************************************/

enum TTypeOfDirections {
	direction_n = 0, direction_ne, direction_e, direction_se, direction_s, direction_sw, direction_w, direction_nw
};

/**********************************************************************************************//**
 * Enum:	TTypesOfLandscapeElement
/**********************************************************************************************//**
/**
\brief Values that represent the types of landscape polygon that are represented in ALMaSS
*/
enum TTypesOfLandscapeElement {
	/** The numbers in comments are the numbers expected in the raw data file direct from GIS input */
	tole_Hedges = 0,  //130
	tole_RoadsideVerge,  //13
	tole_Railway,  //118
	tole_FieldBoundary,  //160
	tole_Marsh,  //95
	tole_Scrub,  //70
	tole_Field,  //20&30
	tole_PermPastureLowYield,  //26
	tole_PermPastureTussocky,  //27
	tole_PermanentSetaside,  //33
	tole_PermPasture,  //35
	tole_NaturalGrassDry,  //110
	tole_RiversidePlants,  //98
	tole_PitDisused,  //75
	tole_RiversideTrees,  //97
	tole_DeciduousForest,  //40
	tole_MixedForest,  //60
	tole_ConiferousForest,  //50
	tole_YoungForest,  //55
	tole_StoneWall,  //15
	tole_Fence, //225
	tole_Garden,  //11//tole20
	tole_Track,  //123
	tole_SmallRoad,  //122
	tole_LargeRoad,  //121
	tole_Building,  //5
	tole_ActivePit,  //115
	tole_Freshwater,  //90
	tole_River,  //96
	tole_Saltwater,  //80
	tole_Coast,  //100//tole30
	tole_HedgeBank,  //140
	tole_BeetleBank,  //141
	tole_Heath,  //94
	tole_Orchard,  //56
	tole_UnsprayedFieldMargin,  //31
	tole_OrchardBand,  //57
	tole_MownGrassStrip,  //58
	tole_BareRock,  //69
	tole_AmenityGrass,  //12
	tole_Parkland,  //14
	tole_UrbanNoVeg,  //8
	tole_UrbanPark,  //17
	tole_BuiltUpWithParkland,  //16
	tole_SandDune,  //101
	tole_Copse,  //41
	tole_RoadsideSlope,  //201
	tole_MetalledPath,  //202
	tole_Carpark,  //203
	tole_Churchyard,  //204
	tole_NaturalGrassWet,  //205
	tole_Saltmarsh,  //206
	tole_Stream,  //207
	tole_HeritageSite,  //208
	tole_UnknownGrass,
	tole_Wasteland, // 209
	tole_IndividualTree,  //
	tole_WoodyEnergyCrop,// 216
	tole_PlantNursery, // 
	tole_Pylon,
	tole_WindTurbine,
	tole_WoodlandMargin,
	tole_Vildtager,
	tole_PermPastureTussockyWet, // 218
	tole_Pond, // 219
	tole_FishFarm, // 220
	tole_UrbanVeg, // Urban vegetated but not garden or park  9
	tole_RiverBed, //221
	tole_DrainageDitch, //222
	tole_Canal, //223
	tole_RefuseSite, // 224
	tole_WaterBufferZone, //226
	tole_Airport,
	tole_Portarea,
	tole_Saltpans,
	tole_Pipeline,
	tole_SwampForest,
	tole_MontadoCorkOak,
	tole_MontadoHolmOak,
	tole_MontadoMixed,
	tole_AgroForestrySystem,
	tole_CorkOakForest,
	tole_HolmOakForest,
	tole_OtherOakForest,
	tole_ChestnutForest,
	tole_EucalyptusForest,
	tole_InvasiveForest,
	tole_MaritimePineForest,
	tole_StonePineForest,
	tole_Vineyard,
	tole_OliveGrove,
	tole_RiceField,
	tole_SolarPanel,
	tole_ForestAisle,
	tole_OOrchard,
	tole_BushFruit,
	tole_OBushFruit,
	tole_ChristmasTrees,
	tole_OChristmasTrees,
	tole_EnergyCrop,
	tole_OEnergyCrop,
	tole_FarmForest,
	tole_OFarmForest,
	tole_PermPasturePigs,
	tole_OPermPasturePigs,
	tole_OPermPasture,
	tole_OPermPastureLowYield,
	tole_FarmYoungForest,
	tole_OFarmYoungForest,
	tole_AlmondPlantation,
	tole_WalnutPlantation,
	tole_FarmBufferZone,
	tole_NaturalFarmGrass,
	tole_GreenFallow,
	tole_FarmFeedingGround,
	tole_FlowersPerm,
	tole_AsparagusPerm,
	tole_MushroomPerm,
	tole_OtherPermCrop,
	tole_OAsparagusPerm,
	tole_FlowerStrip, // 529
	tole_FlowerStripWithRotation,
	tole_Missing, // 2112 will cause this polygon to be removed and its pixels replaced.
	tole_Chameleon, // Special behaviour polygon - replaced when read in, but needs to be here for landscape manipulations
	tole_Foobar 	// Don't use this one when communicating with the landscape model, it will trip an error in most cases!
};

/**********************************************************************************************//**
 * Enum:	TTypesOfVegetation
 **************************************************************************************************/


/** \brief	Values that represent the types of vegetation that are represented in ALMaSS */
enum TTypesOfVegetation {

	// None crop tovs
	tov_None = 0,
	tov_NaturalGrass,
	tov_NoGrowth,
	tov_FlowerStrip1,
	tov_FlowerStrip2,
	tov_FlowerStrip3,
	tov_WaterBufferZone,

	tov_Carrots,
	tov_CloverGrassGrazed1,
	tov_CloverGrassGrazed2,
	tov_FieldPeas,
	tov_FodderBeet,
	tov_Maize,
	tov_Oats,
	tov_OBarleyPeaCloverGrass, 

	tov_OCarrots,
	tov_OCloverGrassGrazed1,
	tov_OCloverGrassGrazed2,
	tov_OCloverGrassSilage1,
	tov_OFieldPeas,
	tov_OFirstYearDanger,
	tov_OGrazingPigs,
	tov_OOats,
	tov_OPermanentGrassGrazed,
	tov_OPotatoes, 

	tov_OSeedGrass1,
	tov_OSeedGrass2,
	tov_OSetAside,
	tov_OSpringBarley,
	tov_OSpringBarleyClover,
	tov_OSpringBarleyGrass,
	tov_OSpringBarleyPigs,
	tov_OTriticale,
	tov_OWinterBarley,
	tov_OWinterRape,

	tov_OWinterRye,
	tov_OWinterWheatUndersown,
	tov_PermanentGrassGrazed,
	tov_PermanentGrassTussocky,
	tov_PermanentSetAside,
	tov_Potatoes,
	tov_PotatoesIndustry,
	tov_SeedGrass1,
	tov_SeedGrass2,
	tov_SetAside, 

	tov_SpringBarley,
	tov_SpringBarleyCloverGrass,
	tov_SpringBarleyGrass, 
	tov_SpringBarleySeed,
	tov_SpringBarleySilage,
	tov_SpringRape, 
	tov_SpringWheat, 
	tov_Triticale, 
	tov_WinterBarley, 
	tov_WinterRape,

	tov_WinterRye, 
	tov_WinterWheat, 
	tov_WWheatPControl, 
	tov_WWheatPToxicControl, 
	tov_WWheatPTreatment, 
	tov_AgroChemIndustryCereal,
	tov_WinterWheatShort, 
	tov_OSBarleySilage, 
	tov_OFieldPeasSilage, 
	tov_FieldPeasStrigling, 

	tov_MaizeStrigling, 
	tov_SpringBarleyStrigling, 
	tov_SpringBarleyCloverGrassStrigling,
	tov_WinterBarleyStrigling, 
	tov_WinterRapeStrigling, 
	tov_WinterRyeStrigling, 
	tov_WinterWheatStrigling,
	tov_SpringBarleyPeaCloverGrassStrigling, 
	tov_YoungForest, 
	tov_SpringBarleyStriglingSingle, 

	tov_SpringBarleyStriglingCulm, 
	tov_WinterWheatStriglingSingle, 
	tov_WinterWheatStriglingCulm, 
	tov_MaizeSilage, 
	tov_FodderGrass,
	tov_Lawn, 
	tov_PermanentGrassLowYield, 
	tov_SpringBarleyPTreatment, 
	tov_OSpringBarleyExt, 
	tov_OWinterWheatUndersownExt,  

	tov_OWinterBarleyExt, 
	tov_OMaizeSilage, 
	tov_SpringBarleySKManagement, 
	tov_FieldPeasSilage, 
	tov_OrchardCrop, 
	tov_OFodderBeet,
	tov_Wasteland, 
	tov_PlantNursery, 
	tov_SugarBeet, 
	tov_SpringBarleySpr, 
	tov_Heath, 
	tov_BroadBeans, 
	tov_OWinterWheat,

	tov_NorwegianOats, 
	tov_NorwegianSpringBarley, 
	tov_NorwegianPotatoes,

	tov_PLWinterWheat, 
	tov_PLWinterRape,
	tov_PLWinterBarley,
	tov_PLWinterTriticale, 
	tov_PLWinterRye,
	tov_PLSpringWheat,
	tov_PLSpringBarley,
	tov_PLMaize,
	tov_PLMaizeSilage,
	tov_PLPotatoes,
	tov_PLBeet,
	tov_PLFodderLucerne1,
	tov_PLFodderLucerne2,
	tov_PLCarrots,
	tov_PLSpringBarleySpr,
	tov_PLWinterWheatLate,
	tov_PLBeetSpr,
	tov_PLBeans,

	tov_NLBeet,
	tov_NLCarrots,
	tov_NLMaize,
	tov_NLPotatoes,
	tov_NLSpringBarley,
	tov_NLWinterWheat,
	tov_NLCabbage,
	tov_NLTulips,
	tov_NLGrassGrazed1,
	tov_NLGrassGrazed2,
	tov_NLPermanentGrassGrazed,
	tov_NLBeetSpring,
	tov_NLCarrotsSpring,
	tov_NLMaizeSpring,
	tov_NLPotatoesSpring,
	tov_NLSpringBarleySpring,
	tov_NLCabbageSpring,
	tov_NLCatchCropPea,
	tov_NLGrassGrazed1Spring,
	tov_NLGrassGrazedLast,
	tov_NLOrchardCrop,
	tov_NLPermanentGrassGrazedExtensive,
	tov_NLGrassGrazedExtensive1,
	tov_NLGrassGrazedExtensive2,
	tov_NLGrassGrazedExtensive1Spring,
	tov_NLGrassGrazedExtensiveLast,

	tov_UKBeans,
	tov_UKBeet,
	tov_UKMaize,
	tov_UKPermanentGrass,
	tov_UKPotatoes,
	tov_UKSpringBarley,
	tov_UKTempGrass,
	tov_UKWinterBarley,
	tov_UKWinterRape,
	tov_UKWinterWheat,

	tov_BEBeet,
	tov_BEBeetSpring,
	tov_BECatchPeaCrop,
	tov_BEGrassGrazed1,
	tov_BEGrassGrazed1Spring,
	tov_BEGrassGrazed2,
	tov_BEGrassGrazedLast,
	tov_BEMaize,
	tov_BEMaizeSpring,
	tov_BEOrchardCrop,
	tov_BEPotatoes,
	tov_BEPotatoesSpring,
	tov_BEWinterBarley,
	tov_BEWinterWheat,
	tov_BEWinterWheatCC,
	tov_BEMaizeCC,
	tov_BEWinterBarleyCC,


	tov_PTPermanentGrassGrazed,	
	tov_PTWinterWheat,
	tov_PTGrassGrazed,
	tov_PTSorghum,
	tov_PTFodderMix,
	tov_PTTurnipGrazed,		
	tov_PTCloverGrassGrazed1,
	tov_PTCloverGrassGrazed2,
	tov_PTTriticale,
	tov_PTOtherDryBeans,
	tov_PTShrubPastures,	
	tov_PTCorkOak,
	tov_PTVineyards,
	tov_PTWinterBarley,
	tov_PTBeans,
	tov_PTWinterRye,		
	tov_PTRyegrass,
	tov_PTYellowLupin,
	tov_PTMaize,
	tov_PTOats,
	tov_PTPotatoes,		
	tov_PTHorticulture,
	tov_PTCabbage,
	tov_PTCabbage_Hort,
	tov_PTMaize_Hort,
	tov_PTOliveGroveTraditional, //190
	tov_PTOliveGroveTradOrganic,	
	tov_PTOliveGroveIntensive,
	tov_PTOliveGroveSuperIntensive,
	tov_PTSetAside,

	tov_GenericCatchCrop,

	tov_DummyCropPestTesting,

	/* New TOV from Danish 2019 data */
	tov_DKOLegume_Peas,
	tov_DKOLegume_Beans,
	tov_DKOLegume_Whole,
	tov_DKSugarBeets,
	tov_DKFodderBeets,
	tov_DKOFodderBeets,
	tov_DKOSugarBeets,
	tov_DKCabbages,
	tov_DKOCabbages,
	tov_DKCarrots,
	tov_DKOLegumeCloverGrass_Whole,
	tov_DKOCarrots,
	tov_DKLegume_Whole,
	tov_DKLegume_Peas,
	tov_DKLegume_Beans,
	tov_DKWinterWheat,
	tov_DKOWinterWheat,
	tov_DKSpringBarley,
	tov_DKSpringBarleyCloverGrass,
	tov_DKOSpringBarley,
	tov_DKOSpringBarleyCloverGrass,
	tov_DKCerealLegume,
	tov_DKOCerealLegume,
	tov_DKCerealLegume_Whole,
	tov_DKOCerealLegume_Whole,
	tov_DKWinterCloverGrassGrazedSown,
	tov_DKCloverGrassGrazed1,
	tov_DKCloverGrassGrazed2,
	tov_DKCloverGrassGrazed3,
	tov_DKOWinterCloverGrassGrazedSown,
	tov_DKOCloverGrassGrazed1,
	tov_DKOCloverGrassGrazed2,
	tov_DKOCloverGrassGrazed3,
	tov_DKSpringFodderGrass,
	tov_DKWinterFodderGrass,
	tov_DKOWinterFodderGrass,
	tov_DKOSpringFodderGrass,
	tov_DKGrazingPigs,
	tov_DKMaize,
	tov_DKMaizeSilage,
	tov_DKMixedVeg,
	tov_DKOGrazingPigs,
	tov_DKOMaize,
	tov_DKOMaizeSilage,
	tov_DKOMixedVeg,
	tov_DKOPotato,
	tov_DKOPotatoIndustry,
	tov_DKOPotatoSeed,
	tov_DKOSeedGrassFescue_Spring,
	tov_DKOSeedGrassFescue1,
	tov_DKOSeedGrassFescue2,
	tov_DKOSeedGrassFescue3,
	tov_DKOSeedGrassRye1,
	tov_DKOSeedGrassRye2,
	tov_DKOSeedGrassRye3,
	tov_DKOSeedGrassRye_Spring,
	tov_DKOSetAside,
	tov_DKOSetAside_AnnualFlower,
	tov_DKOSetAside_PerennialFlower,
	tov_DKOSetAside_SummerMow,
	tov_DKOSpringBarleySilage,
	tov_DKOSpringOats,
	tov_DKOSpringWheat,
tov_DKOVegSeeds,
tov_DKOWinterBarley,
tov_DKOWinterRape,
tov_DKOWinterRye,
tov_DKPotato,
tov_DKPotatoIndustry,
tov_DKPotatoSeed,
tov_DKSeedGrassFescue_Spring,
tov_DKSeedGrassFescue1,
tov_DKSeedGrassFescue2,
tov_DKSeedGrassFescue3,
tov_DKSeedGrassRye1,
tov_DKSeedGrassRye2,
tov_DKSeedGrassRye3,
tov_DKSeedGrassRye_Spring,
tov_DKSetAside,
tov_DKSetAside_SummerMow,
tov_DKSpringBarley_Green,
tov_DKSpringBarleySilage,
tov_DKSpringOats,
tov_DKSpringWheat,
tov_DKUndefined,
tov_DKVegSeeds,
tov_DKWinterBarley,
tov_DKWinterRape,
tov_DKWinterRye,
tov_DKCatchCrop,
tov_DKOCatchCrop,
tov_DKWinterWheat_CC, // CC = crop followed by catch crop
tov_DKOWinterWheat_CC,
tov_DKWinterRye_CC,
tov_DKOWinterRye_CC,
tov_DKOLegume_Beans_CC,
tov_DKOLegume_Peas_CC,
tov_DKOLegume_Whole_CC,
tov_DKSpringOats_CC,
tov_DKOSpringOats_CC,
tov_DKSpringBarley_CC,
tov_DKOSpringBarley_CC,
tov_DKOLupines,
tov_DKOLentils,
tov_DKGrassTussocky_Perm,
tov_DKOptimalFlowerMix1,
tov_DKOptimalFlowerMix2,
tov_DKOptimalFlowerMix3,

// New DK Permanent Crops - data 2019 
tov_DKOOrchardCrop_Perm,
tov_DKBushFruit_Perm1,
tov_DKBushFruit_Perm2,
tov_DKOBushFruit_Perm1,
tov_DKOBushFruit_Perm2,
tov_DKChristmasTrees_Perm,
tov_DKOChristmasTrees_Perm,
tov_DKEnergyCrop_Perm,
tov_DKOEnergyCrop_Perm,
tov_DKFarmForest_Perm,
tov_DKOFarmForest_Perm,
tov_DKGrazingPigs_Perm,
tov_DKOGrazingPigs_Perm,
tov_DKOGrassGrazed_Perm,
tov_DKOGrassLowYield_Perm,
tov_DKFarmYoungForest_Perm,
tov_DKOFarmYoungForest_Perm,
tov_DKPlantNursery_Perm,
tov_DKOrchardCrop_Perm,
tov_DKGrassGrazed_Perm,
tov_DKGrassLowYield_Perm,
tov_DKOrchApple,
tov_DKOrchPear,
tov_DKOrchCherry,
tov_DKOrchOther,
tov_DKOOrchApple,
tov_DKOOrchPear,
tov_DKOOrchCherry,
tov_DKOOrchOther,

tov_DESugarBeet,
tov_DECabbage,
tov_DECarrots,
tov_DEGrasslandSilageAnnual,
tov_DEGreenFallow_1year,
tov_DELegumes,
tov_DEMaize,
tov_DEMaizeSilage,
tov_DEOats,
tov_DEOCabbages,
tov_DEOCarrots,
tov_DEOGrasslandSilageAnnual,
tov_DEOGreenFallow_1year,
tov_DEOLegume,
tov_DEOMaize,
tov_DEOMaizeSilage,
tov_DEOOats,
tov_DEOPeas,
tov_DEOPermanentGrassGrazed,
tov_DEOPotatoes,
tov_DEOSpringRye,
tov_DEOSugarBeet,
tov_DEOTriticale,
tov_DEOWinterBarley,
tov_DEOWinterRape,
tov_DEOWinterRye,
tov_DEOWinterWheat,
tov_DEPeas,
tov_DEPermanentGrassGrazed,
tov_DEPermanentGrassLowYield,
tov_DEOPermanentGrassLowYield,
tov_DEPotatoes,
tov_DEPotatoesIndustry,
tov_DESpringRye,
tov_DETriticale,
tov_DEWinterRye,
tov_DEWinterBarley,
tov_DEWinterRape,
tov_DEWinterWheat,
tov_DEWinterWheatLate,
tov_DEAsparagusEstablishedPlantation,
tov_DEOAsparagusEstablishedPlantation,
tov_DEHerbsPerennial_1year,
tov_DEHerbsPerennial_after1year,
tov_DEOHerbsPerennial_1year,
tov_DEOHerbsPerennial_after1year,
tov_DESpringBarley,
tov_DEOrchard,
tov_DEOOrchard,
tov_DEBushFruitPerm,
tov_DEOBushFruitPerm,

	//New FI crops
	tov_FIWinterWheat,
	tov_FIOWinterWheat,
	tov_FISugarBeet,
	tov_FIStarchPotato_North,
	tov_FIStarchPotato_South,
	tov_FIOStarchPotato_North,
	tov_FIOStarchPotato_South,
	tov_FISpringWheat,
	tov_FIOSpringWheat,
	tov_FITurnipRape,
	tov_FIOTurnipRape,
	tov_FISpringRape,
	tov_FIOSpringRape,
	tov_FIWinterRye,
	tov_FIOWinterRye,
	tov_FIPotato_North,
	tov_FIPotato_South,
	tov_FIOPotato_North,
	tov_FIOPotato_South,
	tov_FIPotatoIndustry_North,
	tov_FIPotatoIndustry_South,
	tov_FIOPotatoIndustry_North,
	tov_FIOPotatoIndustry_South,
	tov_FISpringOats,
	tov_FIOSpringOats,
	tov_FISpringBarley_Malt,
	tov_FIOSpringBarley_Malt,
	tov_FIFabaBean,
	tov_FIOFabaBean,
	tov_FISpringBarley_Fodder,
	tov_FISprSpringBarley_Fodder,
	tov_FIOSpringBarley_Fodder,
	tov_FIGrasslandPasturePerennial1,
	tov_FIGrasslandPasturePerennial2,
	tov_FIGrasslandSilagePerennial1,
	tov_FIGrasslandSilagePerennial2,
	tov_FINaturalGrassland,
	tov_FIFeedingGround,
	tov_FIGreenFallow_1year,
	tov_FIBufferZone,
	tov_FIGrasslandSilageAnnual,
	tov_FICaraway1,
	tov_FICaraway2,
	tov_FIOCaraway1,
	tov_FIOCaraway2,

	//New FI permanent crops
	tov_FINaturalGrassland_Perm,
	tov_FIGreenFallow_Perm,
	tov_FIBufferZone_Perm,

	//New SE crops
	tov_SESpringBarley,
	tov_SEWinterRape_Seed,
	tov_SEWinterWheat,

	//New FR crops
	tov_FRWinterWheat,
	tov_FRWinterBarley,
	tov_FRWinterTriticale,
	tov_FRWinterRape,
	tov_FRMaize,
	tov_FRMaize_Silage,
	tov_FRSpringBarley,
	tov_FRGrassland,
	tov_FRGrassland_Perm,
	tov_FRSpringOats,
	tov_FRSunflower,
	tov_FRSpringWheat,
	tov_FRPotatoes,
	tov_FRSorghum,
	//New IR crops
	tov_IRSpringWheat,
	tov_IRSpringBarley,
	tov_IRSpringOats,
	tov_IRGrassland_no_reseed,
	tov_IRGrassland_reseed,
	tov_IRWinterBarley,
	tov_IRWinterWheat,
	tov_IRWinterOats,

	//new IT crops
	tov_ITGrassland,
	tov_ITOrchard,
	tov_ITOOrchard,

	tov_Undefined // Must be in here and the last one as well!
};

/*******************************************************************************************
 ****************************** Farmimg Enums **********************************************
 *******************************************************************************************

/**********************************************************************************************//**
 * Enum:	TTypeCropClassification
 *
 * Brief:	Values that represent types of crop classifications
 **************************************************************************************************/
enum TTypeCropClassification {
	tocc_Winter = 0,
	tocc_Spring,
	tocc_Catch,
	tocc_Foobar
};

/**********************************************************************************************//**
 * Enum:	TTypesOfFarmObjects
 *
 * Brief:	Values that represent overall of farms treated as separate 'life-stages'
 **************************************************************************************************/
enum TTypesOfFarmObjects
{
	fob_Basic = 0,
	tob_foobar
};

/**********************************************************************************************//**
 * Enum:	TTypesOfFarm
 *
 * Brief:	Values that represent types of basic farm
 **************************************************************************************************/
enum TTypesOfFarm {
	tof_ConventionalCattle = 0,
	tof_ConventionalPig,
	tof_ConventionalPlant,
	tof_OrganicCattle,
	tof_OrganicPig,
	tof_OrganicPlant,
	tof_PTrialControl,
	tof_PTrialTreatment,
	tof_PTrialToxicControl,
	tof_ConvMarginalJord,
	tof_AgroChemIndustryCerealFarm1, //10
	tof_AgroChemIndustryCerealFarm2,
	tof_AgroChemIndustryCerealFarm3,
	tof_NoPesticideBase,
	tof_NoPesticideNoP,
	tof_UserDefinedFarm1, //15
	tof_UserDefinedFarm2,
	tof_UserDefinedFarm3,
	tof_UserDefinedFarm4,
	tof_UserDefinedFarm5,
	tof_UserDefinedFarm6, //20
	tof_UserDefinedFarm7,
	tof_UserDefinedFarm8,
	tof_UserDefinedFarm9,
	tof_UserDefinedFarm10,
	tof_UserDefinedFarm11,
	tof_UserDefinedFarm12,
	tof_UserDefinedFarm13,
	tof_UserDefinedFarm14,
	tof_UserDefinedFarm15,
	tof_UserDefinedFarm16, // 30
	tof_UserDefinedFarm17,
	tof_UserDefinedFarm18,
	tof_UserDefinedFarm19,
	tof_UserDefinedFarm20,
	tof_UserDefinedFarm21,
	tof_UserDefinedFarm22,
	tof_UserDefinedFarm23,
	tof_UserDefinedFarm24,
	tof_UserDefinedFarm25,
	tof_UserDefinedFarm26, // 40
	tof_UserDefinedFarm27,
	tof_UserDefinedFarm28,
	tof_UserDefinedFarm29,
	tof_UserDefinedFarm30,
	tof_UserDefinedFarm31,
	tof_UserDefinedFarm32,
	tof_UserDefinedFarm33,
	tof_UserDefinedFarm34,
	tof_UserDefinedFarm35,
	tof_UserDefinedFarm36, // 50
	tof_OptimisingFarm,
	tof_Foobar
};

/**********************************************************************************************//**
* Enum:	TTypesOfOptFarms
**************************************************************************************************/
/** \brief		Values that represent types of optimising farms
*/
enum TTypesOfOptFarms {
	toof_Other = 0,
	toof_Cattle, // 1
	toof_Plant, // 2
	toof_Pig, // 3
	toof_Foobar // Must be in here and the last one as well!
};

/**********************************************************************************************//**
* Enum:	TTypesOfSoils
**************************************************************************************************/
/** \brief	Values that represent types of soils in ALMaSS */
enum TTypesOfSoils {
	tos_Water = 0,
	tos_Sand,			// 1
	tos_LoamySand,		// 2
	tos_SandyLoam,		// 3
	tos_SandyClayLoam,	// 4
	tos_SiltLoam,		// 5
	tos_SiltyClayLoam,	// 6
	tos_ClayLoam,		// 7
	tos_Loam,			// 8
	tos_Silt,			// 9
	tos_SandyClay,		// 10
	tos_SiltyClay,		// 11
	tos_Clay,			// 12
	tos_HeavyClay,		// 13
	tos_OrganicPeat,	// 14
	tos_Foobar			// Must be in here and the last one as well!
};

/**********************************************************************************************//**
 * Enum:	TTypesOfFarmSize
 *
 * Brief:	Values that represent types of farm sizes
 **************************************************************************************************/
enum TTypesOfFarmSize {
	tofs_Business = 0,
	tofs_Private, // 1
	tofs_Foobar // Must be in here and the last one as well!
};

/**********************************************************************************************//**
 * Enum:	TTypesOfAnimals
 *
 * Brief:	Values that represent types of farm animals
 **************************************************************************************************/
enum TTypesOfFarmAnimals {
	toa_Horse = 0,
	toa_DCow, // 1
	toa_Suckler,
	toa_DCalf,
	toa_MCalf,
	toa_MCattle,
	toa_Sheep,
	toa_Goat,
	toa_So,
	toa_Finisher,
	toa_Piglet,
	toa_Deer,
	toa_Mink,
	toa_EHen,
	toa_MHen,
	toa_Turkey,
	toa_Goose,
	toa_Duck,
	toa_MDuck,
	toa_Ostrich,
	toa_Foobar // Must be in here and the last one as well!
};

/**********************************************************************************************//**
 * Enum:	TTypesOfCrops
 *
 * Brief:	Values that represent types of crops
 **************************************************************************************************/
typedef enum {
	toc_AsparagusEstablishedPlantation = 0,
	toc_Beans,
	toc_Beans_Whole,
	toc_Beet,
	toc_BushFruit,
	toc_Cabbage,
	toc_CabbageSpring,
	toc_Carrots,
	toc_CarrotsSpring,
	toc_CatchCropPea,
	toc_CloverGrassGrazed1,
	toc_CloverGrassGrazed2,
	toc_CloverGrassGrazed3,
	toc_CorkOak,
	toc_DummyCropPestTesting,
	toc_Fallow,
	toc_FarmForest,
	toc_FieldPeas,
	toc_FieldPeasSilage,
	toc_FieldPeasStrigling,
	toc_FodderBeet,
	toc_FodderGrass,
	toc_FodderLucerne1,
	toc_FodderLucerne2,
	toc_GenericCatchCrop,
	toc_GrassGrazed1,
	toc_GrassGrazed2,
	toc_GrassGrazedExtensive,
	toc_GrassGrazedLast,
	toc_GrazingPigs,
	toc_Horticulture,
	toc_Maize,
	toc_MaizeSilage,
	toc_MaizeSpring,
	toc_MaizeStrigling,
	toc_MixedVeg,
	toc_OAsparagusEstablishedPlantation,
	toc_Oats,
	toc_OBarleyPeaCloverGrass,
	toc_OBeans,
	toc_OBeans_Whole,
	toc_OBushFruit,
	toc_OCabbage,
	toc_OCarrots,
	toc_OCloverGrassGrazed1,
	toc_OCloverGrassGrazed2,
	toc_OCloverGrassGrazed3,
	toc_OCloverGrassSilage1,
	toc_OFarmForest,
	toc_OFieldPeas,
	toc_OFieldPeasSilage,
	toc_OFirstYearDanger,
	toc_OFodderBeet,
	toc_OFodderGrass,
	toc_OGrazingPigs,
	toc_OliveGrove,
	toc_OLentils,
	toc_OLupines,
	toc_OMaize,
	toc_OMaizeSilage,
	toc_OMixedVeg,
	toc_OOats,
	toc_OOrchApple,
	toc_OOrchardCrop, 
	toc_OOrchCherry,
	toc_OOrchOther,
	toc_OOrchPear,
	toc_OPermanentGrassGrazed,
	toc_OPermanentGrassLowYield,
	toc_OPotatoes,
	toc_OPotatoesIndustry,
	toc_OPotatoesSeed,
	toc_OrchApple,
	toc_OrchardCrop,
	toc_OrchCherry,
	toc_OrchOther,
	toc_OrchPear,
	toc_ORyeGrass,
	toc_OSBarleySilage,
	toc_OSeedGrass1,
	toc_OSeedGrass2,
	toc_OSetAside,
	toc_OSetAside_Flower,
	toc_OSpringBarley,
	toc_OSpringBarleyCloverGrass,
	toc_OSpringBarleyExtensive,
	toc_OSpringBarleyPigs,
	toc_OSpringBarleyPeaCloverGrass,
	toc_OSpringBarleySilage,
	toc_OSpringRape,
	toc_OSpringRye,
	toc_OSpringWheat,
	toc_OStarchPotato,
	toc_OSugarBeet,
	toc_OTriticale,
	toc_OVegSeeds,
	toc_OWinterBarley,
	toc_OWinterBarleyExtensive,
	toc_OWinterRape,
	toc_OWinterRye,
	toc_OWinterWheat,
	toc_OWinterWheatUndersown,
	toc_OWinterWheatUndersownExtensive,
	toc_OYoungForestCrop,
	toc_PermanentGrassGrazed,
	toc_PermanentGrassLowYield,
	toc_PermanentGrassTussocky,
	toc_PermanentSetAside,
	toc_PlantNursery,
	toc_Potatoes,
	toc_PotatoesIndustry,
	toc_PotatoesSeed,
	toc_PotatoesSpring,
	toc_Ryegrass,
	toc_ORyegrass,
	toc_SeedGrass1,
	toc_SeedGrass2,
	toc_SetAside,
	toc_Sorghum,
	toc_SpringBarley,
	toc_SpringBarleyCloverGrass,
	toc_SpringBarleyPeaCloverGrass,
	toc_SpringBarleySeed,
	toc_SpringBarleySilage,
	toc_SpringRape,
	toc_SpringRye,
	toc_SpringWheat,
	toc_StarchPotato,
	toc_SugarBeet,
	toc_Sunflower,
	toc_Triticale,
	toc_Tulips,
	toc_Turnip,
	toc_Unmanaged,
	toc_Vineyards,
	toc_VegSeeds,
	toc_WinterBarley,
	toc_WinterRape,
	toc_WinterRye,
	toc_WinterTriticale,
	toc_WinterWheat,
	toc_YellowLupin,
	toc_YoungForestCrop,
	toc_None,
	toc_Foobar // Must be in here and the last one as well!
}TTypesOfCrops;

/**********************************************************************************************//**
 * Enum:	TTypesOfParameters
 *
 * Brief:	crop parameters that do not vary with farm type, soil type etc.
 **************************************************************************************************/

typedef enum {
	top_Subsidy = 0,
	top_PriceLM, //1
	top_PriceHerb,
	top_PriceFi,
	top_PriceG,
	top_PriceH,
	top_PriceW,
	top_AlfaHerb,
	top_BetaHerb,
	top_AlfaFi,
	top_BetaFi,
	top_AlfaG,
	top_BetaG,
	top_AlfaH,
	top_BetaH,
	top_AlfaW,
	top_BetaW,
	top_Foobar // Must be in here and the last one as well!
}TTypesOfParameters;

/**********************************************************************************************//**
 * Enum:	TTypesOfLivestockParameters
 *
 * Brief:	Values that represent types of livestock parameters
 **************************************************************************************************/

typedef enum {
	tolp_AUKey = 0,
	tolp_Nusable, //1
	tolp_FUuKey, //2
	tolp_Foobar // Must be in here and the last one as well!
}TTypesOfLivestockParameters;

/**********************************************************************************************//**
 * Enum:	TTypesOfCropVariables
 *
 * Brief:	Values that represent types of crop variables
 **************************************************************************************************/

typedef enum {
	tocv_AreaPercent = 0,
	tocv_AreaHa, //1
	tocv_N,
	tocv_Nt,
	tocv_BIHerb, //2
	tocv_BIFi,
	tocv_BI,
	tocv_Grooming,
	tocv_Hoeing,
	tocv_Weeding,
	tocv_TotalLoss,
	tocv_Response,
	tocv_Income,
	tocv_Costs,
	tocv_GM,
	tocv_Savings,
	tocv_Foobar // Must be in here and the last one as well!
}TTypesOfCropVariables;

/**********************************************************************************************//**
 * Enum:	FarmManagementCategory
 *
 * Brief:	Values that represent main types of crop management
 **************************************************************************************************/

enum FarmManagementCategory : int
{
	fmc_Others = 0,
	fmc_Fertilizer,
	fmc_Insecticide,
	fmc_Herbicide,
	fmc_Fungicide,
	fmc_Cultivation,
	fmc_Cutting,
	fmc_Grazing,
	fmc_Watering,
	fmc_Harvest,
	fmc_Foobar
};


/**********************************************************************************************//**
 * Enum:	TTypeOfFarmerGoal
 *
 * Brief:	Values that represent type of farmer goals
 **************************************************************************************************/
enum TTypeOfFarmerGoal {
	tofg_profit = 0,
	tofg_yield,
	tofg_environment,
	tofg_Foobar
};

/**********************************************************************************************//**
 * Enum:	TypesOfOrchard
 *
 * Brief:	Falues that represent different types of orchard to be used for Orchard attributes
 **************************************************************************************************/
enum TypesOfOrchard {
	torch_Apples = 0,
	torcg_Cherries,
	torch_Pear,
	torch_Other,
	torch_Foobar
};

/**********************************************************************************************//**
 * Enum:	TTypesOfPesticideCategory
 *
 * Brief:	********************************************************************************************************
 *  ****************************** Pesticide Enums
 *  *********************************************************
 *  ********************************************************************************************************.
 **************************************************************************************************/
enum TTypesOfPesticideCategory { insecticide = 0, fungicide, herbicide, testpesticide };

/**********************************************************************************************//**
 * Enum:	TTypesOfPesticide
**************************************************************************************************/

/** \brief	Values that represent types of pesticide actions */
enum TTypesOfPesticide
{
	ttop_NoPesticide = -1, // default
	ttop_AcuteEffects = 0,
	ttop_ReproductiveEffects, //1
	ttop_Vinclozolin, //2
	ttop_ModelinkPesticide, // 3
	ttop_ModelinkPesticide21TWA, // 4
	ttop_GeneticDemo,
	ttop_MultipleEffects, // 6
	ttop_eggshellthinning, // 7
	ttop_AcuteDelayedEffects, // 8
	ttop_AcuteEffectsDoseResponse // 9
};

// enum PlantProtectionProducts removed: pesticides are now indexed by a plain int in [0, g_pest_NoPPPs)
// (the number of eCAGs). The old fixed ppp_1..ppp_10/ppp_foobar list is superseded by the eCAGs model.


#endif
