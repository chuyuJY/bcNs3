/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Marco Miozzo <marco.miozzo@cttc.es>
 * Modification: Dizhi Zhou <dizhi.zhou@gmail.com>    // modify codes related to downlink scheduler
 */

#ifndef FDBET_FF_MAC_SCHEDULER_H
#define FDBET_FF_MAC_SCHEDULER_H

#include <ns3/lte-common.h>
#include <ns3/ff-mac-csched-sap.h>
#include <ns3/ff-mac-sched-sap.h>
#include <ns3/ff-mac-scheduler.h>
#include <vector>
#include <map>
#include <ns3/nstime.h>
#include <ns3/lte-amc.h>
#include <ns3/lte-ffr-sap.h>

// value for SINR outside the range defined by FF-API, used to indicate that there
// is no CQI for this element
#define NO_SINR -5000


#define HARQ_PROC_NUM 8
#define HARQ_DL_TIMEOUT 11

namespace ns3 {


typedef std::vector < uint8_t > DlHarqProcessesStatus_t;
typedef std::vector < uint8_t > DlHarqProcessesTimer_t;
typedef std::vector < DlDciListElement_s > DlHarqProcessesDciBuffer_t;
typedef std::vector < std::vector <struct RlcPduListElement_s> > RlcPduList_t; // vector of the LCs and layers per UE
typedef std::vector < RlcPduList_t > DlHarqRlcPduListBuffer_t; // vector of the 8 HARQ processes per UE

typedef std::vector < UlDciListElement_s > UlHarqProcessesDciBuffer_t;
typedef std::vector < uint8_t > UlHarqProcessesStatus_t;


/// fdbetsFlowPerf_t structure
struct fdbetsFlowPerf_t
{
  Time flowStart; ///< flow start time
  unsigned long totalBytesTransmitted; ///< total bytes transmitted
  unsigned int lastTtiBytesTrasmitted; ///< last total bytes transmitted
  double lastAveragedThroughput; ///< last averaged throughput
};


/**
 * \ingroup ff-api
 * \brief Implements the SCHED SAP and CSCHED SAP for a Frequency Domain Blind Equal Throughput scheduler
 *
 * This class implements the interface defined by the FfMacScheduler abstract class
 */

class FdBetFfMacScheduler : public FfMacScheduler
{
public:
  /**
   * \brief Constructor
   *
   * Creates the MAC Scheduler interface implementation
   */
  FdBetFfMacScheduler ();

  /**
   * Destructor
   */
  virtual ~FdBetFfMacScheduler ();

  // inherited from Object
  virtual void DoDispose (void);
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  // inherited from FfMacScheduler
  virtual void SetFfMacCschedSapUser (FfMacCschedSapUser* s);
  virtual void SetFfMacSchedSapUser (FfMacSchedSapUser* s);
  virtual FfMacCschedSapProvider* GetFfMacCschedSapProvider ();
  virtual FfMacSchedSapProvider* GetFfMacSchedSapProvider ();

  // FFR SAPs
  virtual void SetLteFfrSapProvider (LteFfrSapProvider* s);
  virtual LteFfrSapUser* GetLteFfrSapUser ();

  /// allow MemberCschedSapProvider<FdBetFfMacScheduler> class friend access
  friend class MemberCschedSapProvider<FdBetFfMacScheduler>;
  /// allow MemberSchedSapProvider<FdBetFfMacScheduler> class friend access
  friend class MemberSchedSapProvider<FdBetFfMacScheduler>;

  /**
   * Transmission mode configuration update function
   * \param rnti the RNTI
   * \param txMode the transmit mode
   */
  void TransmissionModeConfigurationUpdate (uint16_t rnti, uint8_t txMode);

private:
  //
  // Implementation of the CSCHED API primitives
  // (See 4.1 for description of the primitives)
  //

  /**
   * CSched cell config request function
   * \param params the CSched cell config request parameters
   */
  void DoCschedCellConfigReq (const struct FfMacCschedSapProvider::CschedCellConfigReqParameters& params);

  /**
   * Csched UE config request function
   * \param params the CSched UE config request parameters
   */
  void DoCschedUeConfigReq (const struct FfMacCschedSapProvider::CschedUeConfigReqParameters& params);

  /**
   * Csched LC config request function
   * \param params the CSched LC config request parameters
   */
  void DoCschedLcConfigReq (const struct FfMacCschedSapProvider::CschedLcConfigReqParameters& params);

  /**
   * CSched LC release request function
   * \param params the CSched LC release request parameters
   */
  void DoCschedLcReleaseReq (const struct FfMacCschedSapProvider::CschedLcReleaseReqParameters& params);

  /**
   * CSched UE release request function
   * \param params the CSChed UE release request parameters
   */
  void DoCschedUeReleaseReq (const struct FfMacCschedSapProvider::CschedUeReleaseReqParameters& params);

  //
  // Implementation of the SCHED API primitives
  // (See 4.2 for description of the primitives)
  //

  /**
   * Sched DL RLC buffer request function
   * \param params the Sched DL RLC buffer request parameters
   */
  void DoSchedDlRlcBufferReq (const struct FfMacSchedSapProvider::SchedDlRlcBufferReqParameters& params);

  /**
   * Sched DL paging buffer request function
   * \param params the Sched DL paging buffer request parameters
   */
  void DoSchedDlPagingBufferReq (const struct FfMacSchedSapProvider::SchedDlPagingBufferReqParameters& params);

  /**
   * Sched DL MAC buffer request function
   * \param params the Sched DL MAC buffer request parameters
   */
  void DoSchedDlMacBufferReq (const struct FfMacSchedSapProvider::SchedDlMacBufferReqParameters& params);

  /**
   * Sched DL trigger request function
   *
   * \param params struct FfMacSchedSapProvider::SchedDlTriggerReqParameters&
   */
  void DoSchedDlTriggerReq (const struct FfMacSchedSapProvider::SchedDlTriggerReqParameters& params);

  /**
   * Sched DL RACH info request function
   * \param params the Sched DL RACH info request parameters
   */
  void DoSchedDlRachInfoReq (const struct FfMacSchedSapProvider::SchedDlRachInfoReqParameters& params);

  /**
   * Sched DL CGI info request function
   * \param params the Sched DL CGI info request parameters
   */
  void DoSchedDlCqiInfoReq (const struct FfMacSchedSapProvider::SchedDlCqiInfoReqParameters& params);

  /**
   * Sched UL trigger request function
   * \param params the Sched UL trigger request parameters
   */
  void DoSchedUlTriggerReq (const struct FfMacSchedSapProvider::SchedUlTriggerReqParameters& params);

  /**
   * Sched UL noise interference request function
   * \param params the Sched UL noise interference request parameters
   */
  void DoSchedUlNoiseInterferenceReq (const struct FfMacSchedSapProvider::SchedUlNoiseInterferenceReqParameters& params);

  /**
   * Sched UL SR info request function
   * \param params the Schedul UL SR info request parameters
   */
  void DoSchedUlSrInfoReq (const struct FfMacSchedSapProvider::SchedUlSrInfoReqParameters& params);

  /**
   * Sched UL MAC control info request function
   * \param params the Sched UL MAC control info request parameters
   */
  void DoSchedUlMacCtrlInfoReq (const struct FfMacSchedSapProvider::SchedUlMacCtrlInfoReqParameters& params);

  /**
   * Sched UL CGI info request function
   * \param params the Sched UL CGI info request parameters
   */
  void DoSchedUlCqiInfoReq (const struct FfMacSchedSapProvider::SchedUlCqiInfoReqParameters& params);


  /**
   * Get RBG size function
   * \param dlbandwidth the DL bandwidth
   * \returns the RBG size
   */
  int GetRbgSize (int dlbandwidth);

  /**
   * LC active per flow function
   * \param rnti the RNTI
   * \returns the LC active per flow
   */
  unsigned int LcActivePerFlow (uint16_t rnti);

  /**
   * Estimate UL SNR
   * \param rnti the RNTI
   * \param rb the RB
   * \returns the UL SINR
   */
  double EstimateUlSinr (uint16_t rnti, uint16_t rb);

  /// Refresh DL CQI maps
  void RefreshDlCqiMaps (void);
  /// Refresh UL CQI maps
  void RefreshUlCqiMaps (void);

  /**
   * Update DL RLC buffer info
   * \param rnti the RNTI
   * \param lcid the LCID
   * \param size the size
   */
  void UpdateDlRlcBufferInfo (uint16_t rnti, uint8_t lcid, uint16_t size);
  /**
   * Update UL RLC buffer info
   * \param rnti the RNTI
   * \param size the size
   */
  void UpdateUlRlcBufferInfo (uint16_t rnti, uint16_t size);

  /**
  * \brief Update and return a new process Id for the RNTI specified
  *
  * \param rnti the RNTI of the UE to be updated
  * \return the process id  value
  */
  uint8_t UpdateHarqProcessId (uint16_t rnti);

  /**
  * \brief Return the availability of free process for the RNTI specified
  *
  * \param rnti the RNTI of the UE to be updated
  * \return the process id  value
  */
  uint8_t HarqProcessAvailability (uint16_t rnti);

  /**
  * \brief Refresh HARQ processes according to the timers
  *
  */
  void RefreshHarqProcesses ();

  Ptr<LteAmc> m_amc; ///< amc

  /**
   * Vectors of UE's LC info
  */
  std::map <LteFlowId_t, FfMacSchedSapProvider::SchedDlRlcBufferReqParameters> m_rlcBufferReq;


  /**
  * Map of UE statistics (per RNTI basis) in downlink
  */
  std::map <uint16_t, fdbetsFlowPerf_t> m_flowStatsDl;

  /**
  * Map of UE statistics (per RNTI basis)
  */
  std::map <uint16_t, fdbetsFlowPerf_t> m_flowStatsUl;

  /**
  * Map of UE's DL CQI P01 received
  */
  std::map <uint16_t,uint8_t> m_p10CqiRxed;

  /**
  * Map of UE's timers on DL CQI P01 received
  */
  std::map <uint16_t,uint32_t> m_p10CqiTimers;

  /**
  * Map of UE's DL CQI A30 received
  */
  std::map <uint16_t,SbMeasResult_s> m_a30CqiRxed;

  /**
  * Map of UE's timers on DL CQI A30 received
  */
  std::map <uint16_t,uint32_t> m_a30CqiTimers;

  /**
  * Map of previous allocated UE per RBG
  * (used to retrieve info from UL-CQI)
  */
  std::map <uint16_t, std::vector <uint16_t> > m_allocationMaps;

  /**
  * Map of UEs' UL-CQI per RBG
  */
  std::map <uint16_t, std::vector <double> > m_ueCqi;

  /**
  * Map of UEs' timers on UL-CQI per RBG
  */
  std::map <uint16_t, uint32_t> m_ueCqiTimers;

  /**
  * Map of UE's buffer status reports received
  */
  std::map <uint16_t,uint32_t> m_ceBsrRxed;

  // MAC SAPs
  FfMacCschedSapUser* m_cschedSapUser; ///< csched sap user
  FfMacSchedSapUser* m_schedSapUser; ///< sched sap user
  FfMacCschedSapProvider* m_cschedSapProvider; ///< csched sap provider
  FfMacSchedSapProvider* m_schedSapProvider; ///< sched sap provider

  // FFR SAPs
  LteFfrSapUser* m_ffrSapUser; ///< ffr sap user
  LteFfrSapProvider* m_ffrSapProvider; ///< ffr sap provider

  // Internal parameters
  FfMacCschedSapProvider::CschedCellConfigReqParameters m_cschedCellConfig; ///< csched cell config


  double m_timeWindow; ///< time window

  uint16_t m_nextRntiUl; ///< RNTI of the next user to be served next scheduling in UL

  uint32_t m_cqiTimersThreshold; ///< # of TTIs for which a CQI can be considered valid

  std::map <uint16_t,uint8_t> m_uesTxMode; ///< txMode of the UEs

  // HARQ attributes
  bool m_harqOn; ///< m_harqOn when false inhibit the HARQ mechanisms (by default active)
  std::map <uint16_t, uint8_t> m_dlHarqCurrentProcessId; ///< DL HARQ current process ID 
  //HARQ status
  // 0: process Id available
  // x>0: process Id equal to `x` transmission count
  std::map <uint16_t, DlHarqProcessesStatus_t> m_dlHarqProcessesStatus; ///< DL HARQ process status
  std::map <uint16_t, DlHarqProcessesTimer_t> m_dlHarqProcessesTimer; ///< DL HARQ process timer
  std::map <uint16_t, DlHarqProcessesDciBuffer_t> m_dlHarqProcessesDciBuffer; ///< DL HARQ process DCI buffer
  std::map <uint16_t, DlHarqRlcPduListBuffer_t> m_dlHarqProcessesRlcPduListBuffer; ///< DL HARQ process RLC PDU List 
  std::vector <DlInfoListElement_s> m_dlInfoListBuffered; ///< DL HARQ retx buffered

  std::map <uint16_t, uint8_t> m_ulHarqCurrentProcessId; ///< UL HARQ current process ID
  //HARQ status
  // 0: process Id available
  // x>0: process Id equal to `x` transmission count
  std::map <uint16_t, UlHarqProcessesStatus_t> m_ulHarqProcessesStatus; ///< UL HARQ process status
  std::map <uint16_t, UlHarqProcessesDciBuffer_t> m_ulHarqProcessesDciBuffer; ///< UL HARQ process DCI Buffer


  // RACH attributes
  std::vector <struct RachListElement_s> m_rachList; ///< rach list
  std::vector <uint16_t> m_rachAllocationMap; ///< rach allocation map
  uint8_t m_ulGrantMcs; ///< MCS for UL grant (default 0)

};

} // namespace ns3

#endif /* FDBET_FF_MAC_SCHEDULER_H */
