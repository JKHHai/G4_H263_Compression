


///////////////////////////////////////////////////////////////////////////
//NOTE: This file has been automatically generated by Vivado.
///////////////////////////////////////////////////////////////////////////
`timescale 1ps/1ps
package canteloupe_axi4stream_vip_0_0_pkg;
import axi4stream_vip_pkg::*;
///////////////////////////////////////////////////////////////////////////
// These parameters are named after the component for use in your verification 
// environment.
///////////////////////////////////////////////////////////////////////////
      parameter canteloupe_axi4stream_vip_0_0_VIP_INTERFACE_MODE     = 2;
      parameter canteloupe_axi4stream_vip_0_0_VIP_SIGNAL_SET         = 8'b00010011;
      parameter canteloupe_axi4stream_vip_0_0_VIP_DATA_WIDTH         = 32;
      parameter canteloupe_axi4stream_vip_0_0_VIP_ID_WIDTH           = 0;
      parameter canteloupe_axi4stream_vip_0_0_VIP_DEST_WIDTH         = 0;
      parameter canteloupe_axi4stream_vip_0_0_VIP_USER_WIDTH         = 0;
      parameter canteloupe_axi4stream_vip_0_0_VIP_USER_BITS_PER_BYTE = 0;
      parameter canteloupe_axi4stream_vip_0_0_VIP_HAS_TREADY         = 1;
      parameter canteloupe_axi4stream_vip_0_0_VIP_HAS_TSTRB          = 0;
      parameter canteloupe_axi4stream_vip_0_0_VIP_HAS_TKEEP          = 0;
      parameter canteloupe_axi4stream_vip_0_0_VIP_HAS_TLAST          = 1;
      parameter canteloupe_axi4stream_vip_0_0_VIP_HAS_ACLKEN         = 0;
      parameter canteloupe_axi4stream_vip_0_0_VIP_HAS_ARESETN        = 1;
///////////////////////////////////////////////////////////////////////////


typedef axi4stream_slv_agent #(canteloupe_axi4stream_vip_0_0_VIP_SIGNAL_SET, 
                        canteloupe_axi4stream_vip_0_0_VIP_DEST_WIDTH,
                        canteloupe_axi4stream_vip_0_0_VIP_DATA_WIDTH,
                        canteloupe_axi4stream_vip_0_0_VIP_ID_WIDTH,
                        canteloupe_axi4stream_vip_0_0_VIP_USER_WIDTH, 
                        canteloupe_axi4stream_vip_0_0_VIP_USER_BITS_PER_BYTE,
                        canteloupe_axi4stream_vip_0_0_VIP_HAS_ARESETN) canteloupe_axi4stream_vip_0_0_slv_t;
      
///////////////////////////////////////////////////////////////////////////
// How to start the verification component
///////////////////////////////////////////////////////////////////////////
//      canteloupe_axi4stream_vip_0_0_slv_t  canteloupe_axi4stream_vip_0_0_slv;
//      initial begin : START_canteloupe_axi4stream_vip_0_0_SLAVE
//        canteloupe_axi4stream_vip_0_0_slv = new("canteloupe_axi4stream_vip_0_0_slv", `canteloupe_axi4stream_vip_0_0_PATH_TO_INTERFACE);
//        canteloupe_axi4stream_vip_0_0_slv.start_slave();
//      end

endpackage : canteloupe_axi4stream_vip_0_0_pkg
