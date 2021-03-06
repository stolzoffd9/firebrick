/* $NoKeywords:$ */
/**
 * @file
 *
 * AMD Family_12 CPB Initialization
 *
 * Enables core performance boost.
 *
 * @xrefitem bom "File Content Label" "Release Content"
 * @e project:      AGESA
 * @e sub-project:  CPU/F12
 * @e \$Revision: 44324 $   @e \$Date: 2010-12-22 17:16:51 +0800 (Wed, 22 Dec 2010) $
 *
 */
/*
 ******************************************************************************
 *
 * Copyright (c) 2011, Advanced Micro Devices, Inc.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Advanced Micro Devices, Inc. nor the names of 
 *       its contributors may be used to endorse or promote products derived 
 *       from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL ADVANCED MICRO DEVICES, INC. BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ******************************************************************************
 */

/*----------------------------------------------------------------------------------------
 *                             M O D U L E S    U S E D
 *----------------------------------------------------------------------------------------
 */
#include "AGESA.h"
#include "amdlib.h"
#include "cpuFamilyTranslation.h"
#include "cpuF12PowerMgmt.h"
#include "GnbRegistersLN.h"
#include "NbSmuLib.h"
#include "cpuFeatures.h"
#include "cpuCpb.h"
#include "OptionFamily12hEarlySample.h"
#include "Filecode.h"
CODE_GROUP (G1_PEICC)
RDATA_GROUP (G1_PEICC)

#define FILECODE PROC_CPU_FAMILY_0X12_F12CPB_FILECODE

/*----------------------------------------------------------------------------------------
 *                   D E F I N I T I O N S    A N D    M A C R O S
 *----------------------------------------------------------------------------------------
 */
//extern F12_ES_CPB_SUPPORT F12EarlySampleCpbSupport;

/*----------------------------------------------------------------------------------------
 *                  T Y P E D E F S     A N D     S T R U C T U R E S
 *----------------------------------------------------------------------------------------
 */

/*----------------------------------------------------------------------------------------
 *           P R O T O T Y P E S     O F     L O C A L     F U N C T I O N S
 *----------------------------------------------------------------------------------------
 */

/*----------------------------------------------------------------------------------------
 *                          E X P O R T E D    F U N C T I O N S
 *----------------------------------------------------------------------------------------
 */

/*---------------------------------------------------------------------------------------*/
/**
 * BSC entry point for checking whether or not CPB is supported.
 *
 * @param[in]  CpbServices             The current CPU's family services.
 * @param[in]  PlatformConfig          Contains the runtime modifiable feature input data.
 * @param[in]  Socket                  Zero based socket number to check.
 * @param[in]  StdHeader               Config handle for library and services.
 *
 * @retval     TRUE                    CPB is supported.
 * @retval     FALSE                   CPB is not supported.
 *
 */
BOOLEAN
STATIC
F12IsCpbSupported (
  IN       CPB_FAMILY_SERVICES    *CpbServices,
  IN       PLATFORM_CONFIGURATION *PlatformConfig,
  IN       UINT32                 Socket,
  IN       AMD_CONFIG_PARAMS      *StdHeader
  )
{
  PCI_ADDR             PciAddress;
  D18F4x15C_STRUCT     CpbControl;

  PciAddress.AddressValue = CPB_CTRL_PCI_ADDR;
  LibAmdPciRead (AccessWidth32, PciAddress, &CpbControl.Value, StdHeader);
  return (BOOLEAN) (CpbControl.Field.NumBoostStates != 0);
}


/*---------------------------------------------------------------------------------------*/
/**
 * BSC entry point for for enabling Core Performance Boost.
 *
 * Set up D18F4x15C[BoostSrc] and start the PDMs according to the BKDG.
 *
 * @param[in]  CpbServices             The current CPU's family services.
 * @param[in]  PlatformConfig          Contains the runtime modifiable feature input data.
 * @param[in]  EntryPoint              Current CPU feature dispatch point.
 * @param[in]  Socket                  Zero based socket number to check.
 * @param[in]  StdHeader               Config handle for library and services.
 *
 * @retval     AGESA_SUCCESS           Always succeeds.
 *
 */
AGESA_STATUS
STATIC
F12InitializeCpb (
  IN       CPB_FAMILY_SERVICES    *CpbServices,
  IN       PLATFORM_CONFIGURATION *PlatformConfig,
  IN       UINT64                 EntryPoint,
  IN       UINT32                 Socket,
  IN       AMD_CONFIG_PARAMS      *StdHeader
  )
{
  PCI_ADDR             PciAddress;
  D18F4x15C_STRUCT     CpbControl;
  SMUx0B_x8580_STRUCT  SMUx0Bx8580;

  if ((EntryPoint & CPU_FEAT_BEFORE_PM_INIT) != 0) {
//    F12EarlySampleCpbSupport.F12CpbInitHook (StdHeader);
    PciAddress.AddressValue = CPB_CTRL_PCI_ADDR;
    LibAmdPciRead (AccessWidth32, PciAddress, &CpbControl.Value, StdHeader);
    CpbControl.Field.BoostSrc = 1;
    IDS_OPTION_HOOK (IDS_CPB_CTRL, &CpbControl.Value, StdHeader);
    LibAmdPciWrite (AccessWidth32, PciAddress, &CpbControl.Value, StdHeader);
  } else if ((EntryPoint & CPU_FEAT_INIT_LATE_END) != 0) {
    // Ensure that the recommended settings have been programmed into SMUx0B_x8580, then
    // interrupt the SMU with service index 12h.
    SMUx0Bx8580.Value = 0;
    SMUx0Bx8580.Field.PdmPeriod = 0x1388;
    SMUx0Bx8580.Field.PdmUnit = 1;
    SMUx0Bx8580.Field.PdmCacEn = 1;
    SMUx0Bx8580.Field.PdmEn = 1;
    NbSmuRcuRegisterWrite (SMUx0B_x8580_ADDRESS, &SMUx0Bx8580.Value, 1, TRUE, StdHeader);
    NbSmuServiceRequest (0x12, TRUE, StdHeader);
  }
  return AGESA_SUCCESS;
}

CONST CPB_FAMILY_SERVICES ROMDATA F12CpbSupport =
{
  0,
  F12IsCpbSupported,
  F12InitializeCpb
};
