// Part of dump1090, a Mode S message decoder for RTLSDR devices.
//
// dump1090.h: main program header
//
// Copyright (c) 2014,2015 Oliver Jowett <oliver@mutability.co.uk>
//
// This file is free software: you may copy, redistribute and/or modify it  
// under the terms of the GNU General Public License as published by the
// Free Software Foundation, either version 2 of the License, or (at your  
// option) any later version.  
//
// This file is distributed in the hope that it will be useful, but  
// WITHOUT ANY WARRANTY; without even the implied warranty of  
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License  
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

// This file incorporates work covered by the following copyright and  
// permission notice:
//
//   Copyright (C) 2012 by Salvatore Sanfilippo <antirez@gmail.com>
//
//   All rights reserved.
//
//   Redistribution and use in source and binary forms, with or without
//   modification, are permitted provided that the following conditions are
//   met:
//
//    *  Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//
//    *  Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//
//   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
//   HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
//   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef __DUMP1090_H
#define __DUMP1090_H

// Default version number, if not overriden by the Makefile
#ifndef MODES_DUMP1090_VERSION
#define MODES_DUMP1090_VERSION     "v1.49-SDRplay"
#endif

#ifndef MODES_DUMP1090_VARIANT
# define MODES_DUMP1090_VARIANT     "dump1090-mutability"
#endif

// ============================= Include files ==========================

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <math.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <ctype.h>
#include <limits.h>

#include "compat/compat.h"

#ifndef __cplusplus
#ifndef bool
typedef int bool;
#define true 1
#define false 0
#endif
#endif

typedef void *HANDLE;

// ============================= #defines ===============================

#define MODES_DEFAULT_PPM          0
#define MODES_DEFAULT_FREQ         1090000000
#define MODES_RTL_BUFFERS          15                         // Number of RTL buffers
#define MODES_RSP_BUFFERS          16						        // must be power of 2 
#define MODES_RTL_BUF_SIZE         (16*16384)                 // 256k
#define MODES_RSP_BUF_SIZE			  (MODES_RTL_BUF_SIZE / 2)
#define MODES_MAG_BUF_SAMPLES      (MODES_RTL_BUF_SIZE / 2)   // Each sample is 2 bytes
#define MODES_MAG_BUFFERS          12                         // Number of magnitude buffers (should be smaller than RTL_BUFFERS for flowcontrol to work)
#define MODES_AUTO_GAIN            -100                       // Use automatic gain
#define MODES_MAX_GAIN             999999                     // Use max available gain
#define MODES_RSP_INITIAL_GR       20
#define MODES_MSG_SQUELCH_DB       4.0                        // Minimum SNR, in dB
#define MODES_MSG_ENCODER_ERRS     3                          // Maximum number of encoding errors



#define MODES_MAX_PHASE_STATS      10

#define MODEAC_MSG_SAMPLES       (25 * 2)                     // include up to the SPI bit
#define MODEAC_MSG_BYTES          2
#define MODEAC_MSG_SQUELCH_LEVEL  0x07FF                      // Average signal strength limit
#define MODEAC_MSG_FLAG          (1<<0)
#define MODEAC_MSG_MODES_HIT     (1<<1)
#define MODEAC_MSG_MODEA_HIT     (1<<2)
#define MODEAC_MSG_MODEC_HIT     (1<<3)
#define MODEAC_MSG_MODEA_ONLY    (1<<4)
#define MODEAC_MSG_MODEC_OLD     (1<<5)

#define MODES_PREAMBLE_US        8              // microseconds = bits
#define MODES_PREAMBLE_SAMPLES  (MODES_PREAMBLE_US       * 2)
#define MODES_PREAMBLE_SIZE     (MODES_PREAMBLE_SAMPLES  * sizeof(uint16_t))
#define MODES_LONG_MSG_BYTES     14
#define MODES_SHORT_MSG_BYTES    7
#define MODES_LONG_MSG_BITS     (MODES_LONG_MSG_BYTES    * 8)
#define MODES_SHORT_MSG_BITS    (MODES_SHORT_MSG_BYTES   * 8)
#define MODES_LONG_MSG_SAMPLES  (MODES_LONG_MSG_BITS     * 2)
#define MODES_SHORT_MSG_SAMPLES (MODES_SHORT_MSG_BITS    * 2)
#define MODES_LONG_MSG_SIZE     (MODES_LONG_MSG_SAMPLES  * sizeof(uint16_t))
#define MODES_SHORT_MSG_SIZE    (MODES_SHORT_MSG_SAMPLES * sizeof(uint16_t))

#define MODES_OS_PREAMBLE_SAMPLES  (20)
#define MODES_OS_PREAMBLE_SIZE     (MODES_OS_PREAMBLE_SAMPLES  * sizeof(uint16_t))
#define MODES_OS_LONG_MSG_SAMPLES  (268)
#define MODES_OS_SHORT_MSG_SAMPLES (135)
#define MODES_OS_LONG_MSG_SIZE     (MODES_LONG_MSG_SAMPLES  * sizeof(uint16_t))
#define MODES_OS_SHORT_MSG_SIZE    (MODES_SHORT_MSG_SAMPLES * sizeof(uint16_t))

#define MODES_OUT_BUF_SIZE         (1500)
#define MODES_OUT_FLUSH_SIZE       (MODES_OUT_BUF_SIZE - 256)
#define MODES_OUT_FLUSH_INTERVAL   (60000)

#define MODES_UNIT_FEET 0
#define MODES_UNIT_METERS 1

#define MODES_USER_LATLON_VALID (1<<0)

#define INVALID_ALTITUDE (-9999)

#define MODES_ACFLAGS_LATLON_VALID   (1<<0)  // Aircraft Lat/Lon is decoded
#define MODES_ACFLAGS_ALTITUDE_VALID (1<<1)  // Aircraft altitude is known
#define MODES_ACFLAGS_HEADING_VALID  (1<<2)  // Aircraft heading is known
#define MODES_ACFLAGS_SPEED_VALID    (1<<3)  // Aircraft speed is known
#define MODES_ACFLAGS_VERTRATE_VALID (1<<4)  // Aircraft vertical rate is known
#define MODES_ACFLAGS_SQUAWK_VALID   (1<<5)  // Aircraft Mode A Squawk is known
#define MODES_ACFLAGS_CALLSIGN_VALID (1<<6)  // Aircraft Callsign Identity
#define MODES_ACFLAGS_EWSPEED_VALID  (1<<7)  // Aircraft East West Speed is known
#define MODES_ACFLAGS_NSSPEED_VALID  (1<<8)  // Aircraft North South Speed is known
#define MODES_ACFLAGS_AOG            (1<<9)  // Aircraft is On the Ground
#define MODES_ACFLAGS_LLEVEN_VALID   (1<<10) // Aircraft Even Lot/Lon is known
#define MODES_ACFLAGS_LLODD_VALID    (1<<11) // Aircraft Odd Lot/Lon is known
#define MODES_ACFLAGS_AOG_VALID      (1<<12) // MODES_ACFLAGS_AOG is valid
#define MODES_ACFLAGS_FS_VALID       (1<<13) // Aircraft Flight Status is known
#define MODES_ACFLAGS_NSEWSPD_VALID  (1<<14) // Aircraft EW and NS Speed is known
#define MODES_ACFLAGS_LATLON_REL_OK  (1<<15) // Indicates it's OK to do a relative CPR
#define MODES_ACFLAGS_REL_CPR_USED   (1<<16) // Lat/lon derived from relative CPR
#define MODES_ACFLAGS_CATEGORY_VALID (1<<17) // Aircraft category is known
#define MODES_ACFLAGS_FROM_MLAT      (1<<18) // Data was derived from multilateration
#define MODES_ACFLAGS_ALTITUDE_HAE_VALID (1<<19) // altitude_hae is valid
#define MODES_ACFLAGS_HAE_DELTA_VALID    (1<<20) // hae_delta is valid
#define MODES_ACFLAGS_FROM_TISB      (1<<21) // Data was derived from TIS-B messages

#define MODES_ACFLAGS_LLEITHER_VALID (MODES_ACFLAGS_LLEVEN_VALID | MODES_ACFLAGS_LLODD_VALID)
#define MODES_ACFLAGS_LLBOTH_VALID   (MODES_ACFLAGS_LLEVEN_VALID | MODES_ACFLAGS_LLODD_VALID)
#define MODES_ACFLAGS_AOG_GROUND     (MODES_ACFLAGS_AOG_VALID    | MODES_ACFLAGS_AOG)

/* Where did a bit of data arrive from? In order of increasing priority */
typedef enum {
	SOURCE_INVALID,        /* data is not valid */
	SOURCE_MLAT,           /* derived from mlat */
	SOURCE_MODE_S,         /* data from a Mode S message, no full CRC */
	SOURCE_MODE_S_CHECKED, /* data from a Mode S message with full CRC */
	SOURCE_TISB,           /* data from a TIS-B extended squitter message */
	SOURCE_ADSB,           /* data from a ADS-B extended squitter message */
} datasource_t;

/* What sort of address is this and who sent it?
 * (Earlier values are higher priority)
 */
typedef enum {
	ADDR_ADSB_ICAO,       /* Mode S or ADS-B, ICAO address, transponder sourced */
	ADDR_ADSB_ICAO_NT,    /* ADS-B, ICAO address, non-transponder */
	ADDR_ADSR_ICAO,       /* ADS-R, ICAO address */
	ADDR_TISB_ICAO,       /* TIS-B, ICAO address */

	ADDR_ADSB_OTHER,      /* ADS-B, other address format */
	ADDR_ADSR_OTHER,      /* ADS-R, other address format */
	ADDR_TISB_TRACKFILE,  /* TIS-B, Mode A code + track file number */
	ADDR_TISB_OTHER,      /* TIS-B, other address format */

	ADDR_UNKNOWN          /* unknown address format */
} addrtype_t;

typedef enum {
	UNIT_FEET,
	UNIT_METERS
} altitude_unit_t;

typedef enum {
	ALTITUDE_BARO,
	ALTITUDE_GNSS
} altitude_source_t;

typedef enum {
	AG_INVALID,
	AG_GROUND,
	AG_AIRBORNE,
	AG_UNCERTAIN
} airground_t;

typedef enum {
	SPEED_GROUNDSPEED,
	SPEED_IAS,
	SPEED_TAS
} speed_source_t;

typedef enum {
	HEADING_TRUE,
	HEADING_MAGNETIC
} heading_source_t;

typedef enum {
	SIL_PER_SAMPLE, SIL_PER_HOUR
} sil_type_t;

typedef enum {
	CPR_SURFACE, CPR_AIRBORNE, CPR_COARSE
} cpr_type_t;

#define MODES_NON_ICAO_ADDRESS       (1<<24) // Set on addresses to indicate they are not ICAO addresses

#define MODES_DEBUG_DEMOD (1<<0)
#define MODES_DEBUG_DEMODERR (1<<1)
#define MODES_DEBUG_BADCRC (1<<2)
#define MODES_DEBUG_GOODCRC (1<<3)
#define MODES_DEBUG_NOPREAMBLE (1<<4)
#define MODES_DEBUG_NET (1<<5)
#define MODES_DEBUG_JS (1<<6)

// When debug is set to MODES_DEBUG_NOPREAMBLE, the first sample must be
// at least greater than a given level for us to dump the signal.
#define MODES_DEBUG_NOPREAMBLE_LEVEL 25

#define MODES_INTERACTIVE_REFRESH_TIME 250      // Milliseconds
#define MODES_INTERACTIVE_ROWS          22      // Rows on screen
#define MODES_INTERACTIVE_DISPLAY_TTL 60000     // Delete from display after 60 seconds

#define MODES_NET_HEARTBEAT_INTERVAL 60000      // milliseconds

#define MODES_CLIENT_BUF_SIZE  1024
#define MODES_NET_SNDBUF_SIZE (1024*64)
#define MODES_NET_SNDBUF_MAX  (7)

#define HISTORY_SIZE 120
#define HISTORY_INTERVAL 30000

#define MODES_NOTUSED(V) ((void) V)

#define MAX_AMPLITUDE 65535.0
#define MAX_POWER (MAX_AMPLITUDE * MAX_AMPLITUDE)

#define RSP_ACC_SHIFT 13                // sets time constant of averaging filter
#define RSP_MIN_GAIN_THRESH 128 //512         // increase gain if peaks below this
#define RSP_MAX_GAIN_THRESH 256 //1024        // decrease gain if peaks above this

#include "win_time.h"

#ifdef __cplusplus
extern "C" {
#endif

   // Include subheaders after all the #defines are in place

#include "util.h"
#include "anet.h"
#include "net_io.h"
#include "crc.h"
#include "demod_2000.h"
#include "demod_8000.h"
#include "stats.h"
#include "cpr.h"
#include "icao_filter.h"

//======================== structure declarations =========================

// Structure representing one magnitude buffer
struct mag_buf {
    uint16_t       *data;            // Magnitude data. Starts with Modes.trailing_samples worth of overlap from the previous block
    unsigned        length;          // Number of valid samples _after_ overlap. Total buffer length is buf->length + Modes.trailing_samples.
    uint64_t        sampleTimestamp; // Clock timestamp of the start of this block, 12MHz clock
    struct timespec sysTimestamp;    // Estimated system time at start of block
    uint32_t        dropped;         // Number of dropped samples preceding this buffer
    double          total_power;     // Sum of per-sample input power (in the range [0.0,1.0] per sample), or 0 if not measured
};

// Program global state
struct modes {                             // Internal state
	unsigned        sdr_buf_size;                         // sample buffer size
	int             mag_buf_samples;                      // Number of magnitude samples
	int             mag_buf_num;                          // Number of magnitude buffers
	struct mag_buf *mag_buffers;                          // Converted magnitude buffers from SDR or file input
	unsigned        first_free_buffer;                    // Entry in mag_buffers that will next be filled with input.
	unsigned        first_filled_buffer;                  // Entry in mag_buffers that has valid data and will be demodulated next. If equal to next_free_buffer, there is no unprocessed data.
	struct timespec reader_cpu_accumulator;               // CPU time used by the reader thread, copied out and reset by the main thread under the mutex
	unsigned        trailing_samples;                     // extra trailing samples in magnitude buffers

	uint16_t       *log10lut;        // Magnitude -> log10 lookup table
	int             exit;            // Exit from the main loop when true

    // Sample conversion
    int            dc_filter;        // should we apply a DC filter?
    int            measure_noise;    // should we measure noise power?
    struct converter_state *converter_state;

	/* Input selection flags */
	int           net_only;          // Enable just networking

    // SDRplay
   void          *sdrplay_data;
   int           systemGain;
   int           gr;
   int           max_sig;
   unsigned int  data_index;
   
   // FILE specific
   char         *filename;          // Input from file
   int           fd;                // --ifile option file descriptor

	double        sample_rate;       // actual sample rate in use (in hz)
	int           freq;              // frequency

    // demod_8000
    int         *d8m_dbuf;                              // main data buffer

    int         d8m_phase_av_acc;                       // lowpass match memory
    int         d8m_backtrack_phase_av_acc;             // saved version in case of backtrack
    int         d8m_window;                             // current index in match window
    int         d8m_win_start;                          // start of current match window
    int         d8m_start_phase;                        // intial phase chosen for match
    int         d8m_phase[D8M_NUM_PHASES];              // sliding window for each phase
    int         d8m_backtrack_phase[D8M_NUM_PHASES];    // saved version in case of backtrack

    int         *d8m_match_ar;                          // match values over current window
    int         *d8m_phase_ar;                          // best phase choices over current window

    // Networking
    char           aneterr[ANET_ERR_LEN];
    struct net_service *services;    // Active services
    struct client *clients;          // Our clients

    struct net_writer raw_out;       // Raw output
    struct net_writer beast_out;     // Beast-format output
    struct net_writer sbs_out;       // SBS-format output
    struct net_writer fatsv_out;     // FATSV-format output

#ifdef _WIN32
    WSADATA        wsaData;          // Windows socket initialisation
	int            isSocket;         // Windows treats sockets and files differently
#endif

	// Configuration
	int   nfix_crc;                  // Number of crc bit error(s) to correct
	int   check_crc;                 // Only display messages with good CRC
	int   raw;                       // Raw output format
	int   mode_ac;                   // Enable decoding of SSR Modes A & C
	int   debug;                     // Debugging mode
	int   net;                       // Enable networking
	uint64_t net_heartbeat_interval; // TCP heartbeat interval (milliseconds)
	int   net_output_flush_size;     // Minimum Size of output data
	uint64_t net_output_flush_interval; // Maximum interval (in milliseconds) between outputwrites
	char *net_output_raw_ports;      // List of raw output TCP ports
	char *net_input_raw_ports;       // List of raw input TCP ports
	char *net_output_sbs_ports;      // List of SBS output TCP ports
	char *net_input_beast_ports;     // List of Beast input TCP ports
	char *net_output_beast_ports;    // List of Beast output TCP ports
	char *net_http_ports;            // List of HTTP ports
	char *net_bind_address;          // Bind address
	int   net_sndbuf_size;           // TCP output buffer size (64Kb * 2^n)
	int   net_verbatim;              // if true, send the original message, not the CRC-corrected one
	int   forward_mlat;              // allow forwarding of mlat messages to output ports
	int   quiet;                     // Suppress stdout
	uint32_t show_only;              // Only show messages from this ICAO
	int   interactive;               // Interactive mode
	int   interactive_rows;          // Interactive mode: max number of rows
	uint64_t interactive_display_ttl;// Interactive mode: TTL display
	uint64_t stats;                  // Interval (millis) between stats dumps,
	int   stats_range_histo;         // Collect/show a range histogram?
	int   onlyaddr;                  // Print only ICAO addresses
	int   metric;                    // Use metric units
	int   use_hae;                   // Use HAE altitudes with H suffix when available
	int   mlat;                      // Use Beast ascii format for raw data output, i.e. @...; iso *...;
	int   interactive_rtl1090;       // flight table in interactive mode is formatted like RTL1090
	char *json_dir;                  // Path to json base directory, or NULL not to write json.
	uint64_t json_interval;          // Interval between rewriting the json aircraft file, in milliseconds; also the advertised map refresh interval
	char *html_dir;                  // Path to www base directory.
	int   json_location_accuracy;    // Accuracy of location metadata: 0=none, 1=approx, 2=exact
	int   throttle;                  // When reading from a file, throttle file playback to realtime?
    
   int   oversample;
   int   oversampleDecMode;

   int   json_aircraft_history_next;
    struct {
        char *content;
        int clen;
    } json_aircraft_history[HISTORY_SIZE];

    // User details
    double fUserLat;                // Users receiver/antenna lat/lon needed for initial surface location
    double fUserLon;                // Users receiver/antenna lat/lon needed for initial surface location
    int    bUserFlags;              // Flags relating to the user details
    double maxRange;                // Absolute maximum decoding range, in *metres*

    // State tracking
    struct aircraft *aircrafts;

    // Statistics
    struct stats stats_current;
    struct stats stats_alltime;
    struct stats stats_periodic;
    struct stats stats_1min[15];
    int stats_latest_1min;
    struct stats stats_5min;
    struct stats stats_15min;
};

// The struct we use to store information about a decoded message.
struct modesMessage {
    // Generic fields
    unsigned char msg[MODES_LONG_MSG_BYTES];      // Binary message.
    unsigned char verbatim[MODES_LONG_MSG_BYTES]; // Binary message, as originally received before correction
    int           msgbits;                        // Number of bits in message 
    int           msgtype;                        // Downlink format #
    uint32_t      crc;                            // Message CRC
    int           correctedbits;                  // No. of bits corrected 
    uint32_t      addr;                           // Address Announced
    uint64_t      timestampMsg;                   // Timestamp of the message (12MHz clock)
    struct timespec sysTimestampMsg;              // Timestamp of the message (system time)
    int           remote;                         // If set this message is from a remote station
    double        signalLevel;                    // RSSI, in the range [0..1], as a fraction of full-scale power
    int           score;                          // Scoring from scoreModesMessage, if used

    // DF 11, DF 17
    int  ca;                    // Responder capabilities
    int  iid;

    // DF 17, DF 18
    int    metype;              // Extended squitter message type.
    int    mesub;               // Extended squitter message subtype.
    int    heading;             // Reported by aircraft, or computed from from EW and NS velocity
    int    raw_latitude;        // Non decoded latitude.
    int    raw_longitude;       // Non decoded longitude.
    unsigned nuc_p;             // NUCp value implied by message type
    double fLat;                // Coordinates obtained from CPR encoded data if/when decoded
    double fLon;                // Coordinates obtained from CPR encoded data if/when decoded
    char   flight[16];          // 8 chars flight number.
    int    ew_velocity;         // E/W velocity.
    int    ns_velocity;         // N/S velocity.
    int    vert_rate;           // Vertical rate.
    int    velocity;            // Reported by aircraft, or computed from from EW and NS velocity
    unsigned category;          // A0 - D7 encoded as a single hex byte
    int    altitude_hae;        // altitude reported as GNSS HAE
    int    hae_delta;           // difference between HAE and baro alt

    // DF 18
    int    cf;                  // Control Field

    // DF4, DF5, DF20, DF21
    int  fs;                    // Flight status for DF4,5,20,21
    int  modeA;                 // 13 bits identity (Squawk).

    // DF20, DF21
    int  bds;                   // BDS value implied if overlay control was used

    // Fields used by multiple message types.
    int  altitude;
    int  unit; 
    int  bFlags;                // Flags related to fields in this structure
};

// This one needs modesMessage:
#include "track.h"

// ======================== function declarations =========================

//
// Functions exported from mode_ac.c
//
int detectModeA(uint16_t *m, struct modesMessage *mm);
void decodeModeAMessage(struct modesMessage *mm, int ModeA);
int ModeAToModeC(unsigned int ModeA);

//
// Functions exported from mode_s.c
//
int modesMessageLenByType(int type);
int scoreModesMessage(unsigned char *msg, int validbits);
int decodeModesMessage(struct modesMessage *mm, unsigned char *msg);
void displayModesMessage(struct modesMessage *mm);
void useModesMessage(struct modesMessage *mm);

//
// Functions exported from interactive.c
//
void interactiveShowData(void);
int GetInteractiveData(char ** sub_str);

extern struct modes Modes;

#ifdef __cplusplus
}
#endif

#endif // __DUMP1090_H

