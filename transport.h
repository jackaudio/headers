/*
    Copyright (C) 2002 Paul Davis
    Copyright (C) 2003 Jack O'Quin

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 2.1 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#ifndef JACK_TRANSPORT_H
#define JACK_TRANSPORT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <jack/types.h>
#include <jack/weakmacros.h>

/**
 * Transport states.
 */
typedef enum {

	/* the order matters for binary compatibility */
	JackTransportStopped = 0,	/**< Transport halted */
	JackTransportRolling = 1,	/**< Transport playing */
	JackTransportLooping = 2,	/**< For OLD_TRANSPORT, now ignored */
	JackTransportStarting = 3	/**< Waiting for sync ready */

} jack_transport_state_t;

typedef uint64_t jack_unique_t;		/**< Unique ID (opaque) */

/**
 * Optional struct jack_position_t fields.
 */
typedef enum {
	JackPositionBBT      = 0x10,  /**< Bar, Beat, Tick */
	JackPositionTimecode = 0x20,  /**< External timecode */
	JackBBTFrameOffset   = 0x40,  /**< Frame offset of BBT information */
	JackAudioVideoRatio  = 0x80,  /**< audio frames per video frame */
	JackVideoFrameOffset = 0x100, /**< frame offset of first video frame */
	JackTickDouble       = 0x200, /**< double-resolution tick */
} jack_position_bits_t;

/** all valid position bits */
#define JACK_POSITION_MASK (JackPositionBBT|JackPositionTimecode|JackBBTFrameOffset|JackAudioVideoRatio|JackVideoFrameOffset)
#define EXTENDED_TIME_INFO

/** transport tick_double member is available for use */
#define JACK_TICK_DOUBLE

/**
 * Struct for transport position information.
 */
typedef struct {

    /*@{*/
    /** @name Server-set fields
     * these cannot be set from clients; the server sets them */
    jack_unique_t	unique_1;	/**< unique ID */
    jack_time_t		usecs;		/**< microsecond timestamp that is guaranteed to be
                                             monotonic, but not neccessarily
                                             linear.

                                             The absolute value is
                                             implementation-dependent (i.e. it
                                             could be wall-clock, time since
                                             jack started, uptime, etc). */
    jack_nframes_t	frame_rate;	/**< current frame rate, in frames per second */
    /*}@*/

    /*@{*/
    /** @name Mandatory fields
      */
    jack_nframes_t	frame;		/**< frame number, always present/required.

                                             This is the frame number on the
                                             transport timeline, which is not
                                             the same as what @ref
                                             jack_frame_time returns. */
    jack_position_bits_t valid;		/**< which other fields are valid, as a
                                             bitmask constructed from values in
                                             \ref jack_position_bits_t */
    /*}@*/

    /*@{*/
    /** @name JackPositionBBT fields
     * Bar:Beat.Tick-related information.
     *
     * Applications that support
     * JackPositionBBT are encouraged to also fill the JackBBTFrameOffset
     */
    int32_t		bar;		/**< current bar

                                             Should be >0: the first bar is
                                             bar '1'. */
    int32_t		beat;		/**< current beat-within-bar

                                             Should be >0 and <=beats_per_bar:
                                             the first beat is beat '1'.
                                             */
    int32_t		tick;		/**< current tick-within-beat

                                             Should be >= 0 and < ticks_per_beat:
                                             the first tick is tick '0'. */
    double		bar_start_tick; /**< number of ticks that have elapsed
                                             between frame 0 and the first beat
                                             of the current measure. */

    float		beats_per_bar;	/**< time signature "numerator" */
    float		beat_type;	/**< time signature "denominator" */
    double		ticks_per_beat; /**< number of ticks within a beat.

                                             Usually a moderately large integer
                                             with many denominators, such as
                                             1920.0 */
    double		beats_per_minute; /**< BPM, quantized to block size. This
                                             means when the tempo is not constant
                                             within this block, the BPM value should
                                             adapted to compensate for this. This
                                             is different from most fields in this
                                             struct, which specify the value at
                                             the beginning of the block rather
                                             than an average.*/
    /*}@*/

    /*@{*/
    /** @name JackPositionTimecode fields
     * EXPERIMENTAL: could change */
    double		frame_time;	/**< current time in seconds */
    double		next_time;	/**< next sequential frame_time
					     (unless repositioned) */
    /*}@*/

    /*@{*/
    /* JackBBTFrameOffset fields */
    jack_nframes_t	bbt_offset;	/**< frame offset for the BBT fields
					     (the given bar, beat, and tick
					     values actually refer to a time
					     frame_offset frames before the
					     start of the cycle), should
					     be assumed to be 0 if
					     JackBBTFrameOffset is not
					     set. If JackBBTFrameOffset is
					     set and this value is zero, the BBT
					     time refers to the first frame of this
					     cycle. If the value is positive,
					     the BBT time refers to a frame that
					     many frames before the start of the
					     cycle. */
    /*}@*/

    /*@{*/
    /* JACK video positional data
     * EXPERIMENTAL: could change */
    float               audio_frames_per_video_frame; /**< number of audio frames
					     per video frame. Should be assumed
					     zero if JackAudioVideoRatio is not
					     set. If JackAudioVideoRatio is set
					     and the value is zero, no video
					     data exists within the JACK graph */

    jack_nframes_t      video_offset;   /**< audio frame at which the first video
					     frame in this cycle occurs. Should
					     be assumed to be 0 if JackVideoFrameOffset
					     is not set. If JackVideoFrameOffset is
					     set, but the value is zero, there is
					     no video frame within this cycle. */
    /*}@*/

    /*@{*/
    /** @name JACK Extra transport fields */
    double              tick_double; /**< current tick-within-beat in double resolution.
					     Should be assumed zero if JackTickDouble is not set.
					     Since older versions of JACK do not expose this variable,
					     the macro JACK_TICK_DOUBLE is provided,
					     which can be used as build-time detection. */
    /*}@*/

    /*@{*/
    /** @name Other fields */
    /* For binary compatibility, new fields should be allocated from
     * this padding area with new valid bits controlling access, so
     * the existing structure size and offsets are preserved. */
    int32_t		padding[5];
    /*}@*/

    /* When (unique_1 == unique_2) the contents are consistent. */
    jack_unique_t	unique_2;	/**< unique ID */

} POST_PACKED_STRUCTURE jack_position_t;

/**
 * @defgroup TransportControl Transport and Timebase control
 * @{
 */

/**
 * Called by the timebase master to release itself from that
 * responsibility.
 *
 * If the timebase master releases the timebase or leaves the JACK
 * graph for any reason, the JACK engine takes over at the start of
 * the next process cycle.  The transport state does not change.  If
 * rolling, it continues to play, with frame numbers as the only
 * available position information.
 *
 * @see jack_set_timebase_callback
 *
 * @param client the JACK client structure.
 *
 * @return 0 on success, otherwise a non-zero error code.
 */
int  jack_release_timebase (jack_client_t *client) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * Prototype for the @a sync_callback defined by @ref slowsyncclients
 * "slow-sync clients". When the client is active, this callback is
 * invoked just before process() in the same thread.  This occurs once
 * after registration, then subsequently whenever some client requests
 * a new position, or the transport enters the ::JackTransportStarting
 * state.  This realtime function must not wait.
 *
 * The transport @a state will be:
 *
 *   - ::JackTransportStopped when a new position is requested;
 *   - ::JackTransportStarting when the transport is waiting to start;
 *   - ::JackTransportRolling when the timeout has expired, and the
 *   position is now a moving target.
 *
 * @param state current transport state.
 * @param pos new transport position.
 * @param arg the argument supplied by jack_set_sync_callback().
 *
 * @return TRUE (non-zero) when ready to roll.
 */
typedef int  (*JackSyncCallback)(jack_transport_state_t state,
				 jack_position_t *pos,
				 void *arg);

/**
 * Register (or unregister) as a @ref slowsyncclients "slow-sync client", that cannot
 * respond immediately to transport position changes.
 *
 * The @a sync_callback will be invoked at the first available
 * opportunity after its registration is complete.  If the client is
 * currently active this will be the following process cycle,
 * otherwise it will be the first cycle after calling jack_activate().
 * After that, it runs according to the ::JackSyncCallback rules.
 * Clients that don't set a @a sync_callback are assumed to be ready
 * immediately any time the transport wants to start.
 *
 * @param client the JACK client structure.
 * @param sync_callback is a realtime function that returns TRUE when
 * the client is ready.  Setting @a sync_callback to NULL declares that
 * this client no longer requires slow-sync processing.
 * @param arg an argument for the @a sync_callback function.
 *
 * @return 0 on success, otherwise a non-zero error code.
 */
int  jack_set_sync_callback (jack_client_t *client,
			     JackSyncCallback sync_callback,
			     void *arg) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * Set the timeout value for @ref slowsyncclients "slow-sync clients".
 *
 * This timeout prevents unresponsive slow-sync clients from
 * completely halting the transport mechanism.  The default is two
 * seconds.  When the timeout expires, the transport starts rolling,
 * even if some slow-sync clients are still unready.  The @a
 * sync_callbacks of these clients continue being invoked, giving them
 * a chance to catch up.
 *
 * @see jack_set_sync_callback
 *
 * @param client the JACK client structure.
 * @param timeout is delay (in microseconds) before the timeout expires.
 *
 * @return 0 on success, otherwise a non-zero error code.
 */
int  jack_set_sync_timeout (jack_client_t *client,
			    jack_time_t timeout) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * Prototype for the @a timebase_callback used to provide extended
 * position information.  Its output affects all of the following
 * process cycle.  This realtime function must not wait.
 *
 * This function is called immediately after process() in the same
 * thread whenever the transport is rolling, or when any client has
 * requested a new position in the previous cycle.  The first cycle
 * after jack_set_timebase_callback() is also treated as a new
 * position, or the first cycle after jack_activate() if the client
 * had been inactive.
 *
 * The timebase master may not use its @a pos argument to set @a
 * pos->frame.  To change position, use jack_transport_reposition() or
 * jack_transport_locate().  These functions are realtime-safe, the @a
 * timebase_callback can call them directly.
 *
 * @param state current transport state.
 * @param nframes number of frames in current period.
 * @param pos address of the position structure for the next cycle; @a
 * pos->frame will be its frame number.  If @a new_pos is FALSE, this
 * structure contains extended position information from the current
 * cycle.  If TRUE, it contains whatever was set by the requester.
 * The @a timebase_callback's task is to update the extended
 * information here.
 * @param new_pos TRUE (non-zero) for a newly requested @a pos, or for
 * the first cycle after the @a timebase_callback is defined.
 * @param arg the argument supplied by jack_set_timebase_callback().
 */
typedef void (*JackTimebaseCallback)(jack_transport_state_t state,
				     jack_nframes_t nframes,
				     jack_position_t *pos,
				     int new_pos,
				     void *arg);

/**
 * Register as timebase master for the JACK subsystem.
 *
 * The timebase master registers a callback that updates extended
 * position information such as beats or timecode whenever necessary.
 * Without this extended information, there is no need for this
 * function.
 *
 * There is never more than one master at a time.  When a new client
 * takes over, the former @a timebase_callback is no longer called.
 * Taking over the timebase may be done conditionally, so it fails if
 * there was a master already.
 *
 * The method may be called whether the client has been activated or not.
 *
 * @param client the JACK client structure.
 * @param conditional non-zero for a conditional request.
 * @param timebase_callback is a realtime function that returns
 * position information.
 * @param arg an argument for the @a timebase_callback function.
 *
 * @return
 *   - 0 on success;
 *   - EBUSY if a conditional request fails because there was already a
 *   timebase master;
 *   - other non-zero error code.
 */
int  jack_set_timebase_callback (jack_client_t *client,
				 int conditional,
				 JackTimebaseCallback timebase_callback,
				 void *arg) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * Reposition the transport to a new frame number.
 *
 * May be called at any time by any client.  The new position takes
 * effect in two process cycles.  If there are @ref slowsyncclients
 * "slow-sync clients" and the transport is already rolling, it will
 * enter the ::JackTransportStarting state and begin invoking their @a
 * sync_callbacks until ready.  This function is realtime-safe.
 *
 * @see jack_transport_reposition, jack_set_sync_callback
 *
 * @param client the JACK client structure.
 * @param frame frame number of new transport position.
 *
 * @return 0 if valid request, non-zero otherwise.
 */
int  jack_transport_locate (jack_client_t *client,
			    jack_nframes_t frame) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * Query the current transport state and position.
 *
 * This function is realtime-safe, and can be called from any thread.
 * If called from the process thread, @a pos corresponds to the first
 * frame of the current cycle and the state returned is valid for the
 * entire cycle.
 *
 * @param client the JACK client structure.
 * @param pos pointer to structure for returning current transport
 * position; @a pos->valid will show which fields contain valid data.
 * If @a pos is NULL, do not return position information.
 *
 * @return Current transport state.
 */
jack_transport_state_t jack_transport_query (const jack_client_t *client,
					     jack_position_t *pos) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * Return an estimate of the current transport frame,
 * including any time elapsed since the last transport
 * positional update.
 *
 * @param client the JACK client structure
 */
jack_nframes_t jack_get_current_transport_frame (const jack_client_t *client) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * Request a new transport position.
 *
 * May be called at any time by any client.  The new position takes
 * effect in two process cycles.  If there are @ref slowsyncclients
 * "slow-sync clients" and the transport is already rolling, it will
 * enter the ::JackTransportStarting state and begin invoking their @a
 * sync_callbacks until ready.  This function is realtime-safe.
 *
 * @see jack_transport_locate, jack_set_sync_callback
 *
 * @param client the JACK client structure.
 * @param pos requested new transport position. Fill pos->valid to specify
 *   which fields should be taken into account. If you mark a set of fields
 *   as valid, you are expected to fill them all.
 *
 * @return 0 if valid request, EINVAL if position structure rejected.
 */
int  jack_transport_reposition (jack_client_t *client,
				const jack_position_t *pos) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * Start the JACK transport rolling.
 *
 * Any client can make this request at any time.  It takes effect no
 * sooner than the next process cycle, perhaps later if there are
 * @ref slowsyncclients "slow-sync clients".  This function is realtime-safe.
 *
 * @see jack_set_sync_callback
 *
 * @param client the JACK client structure.
 */
void jack_transport_start (jack_client_t *client) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * Stop the JACK transport.
 *
 * Any client can make this request at any time.  It takes effect on
 * the next process cycle.  This function is realtime-safe.
 *
 * @param client the JACK client structure.
 */
void jack_transport_stop (jack_client_t *client) JACK_OPTIONAL_WEAK_EXPORT;

/*@}*/

/*********************************************************************
 * The following interfaces are DEPRECATED.  They are only provided
 * for compatibility with the earlier JACK transport implementation.
 *********************************************************************/

/**
 * Optional struct jack_transport_info_t fields.
 *
 * @see jack_position_bits_t.
 */
typedef enum {

	JackTransportState =    0x1,	/**< Transport state */
	JackTransportPosition = 0x2,	/**< Frame number */
	JackTransportLoop =     0x4,	/**< Loop boundaries (ignored) */
	JackTransportSMPTE =    0x8,	/**< SMPTE (ignored) */
	JackTransportBBT =      0x10	/**< Bar, Beat, Tick */

} jack_transport_bits_t;

/**
 * Deprecated struct for transport position information.
 *
 * @deprecated This is for compatibility with the earlier transport
 * interface.  Use the jack_position_t struct, instead.
 */
typedef struct {

    /* these two cannot be set from clients: the server sets them */

    jack_nframes_t frame_rate;		/**< current frame rate (per second) */
    jack_time_t    usecs;		/**< monotonic, free-rolling */

    jack_transport_bits_t  valid;	/**< which fields are legal to read */
    jack_transport_state_t transport_state;
    jack_nframes_t         frame;
    jack_nframes_t         loop_start;
    jack_nframes_t         loop_end;

    long           smpte_offset;	/**< SMPTE offset (from frame 0) */
    float          smpte_frame_rate;	/**< 29.97, 30, 24 etc. */

    int            bar;
    int            beat;
    int            tick;
    double         bar_start_tick;

    float          beats_per_bar;
    float          beat_type;
    double         ticks_per_beat;
    double         beats_per_minute;

} jack_transport_info_t;

/**
 * Gets the current transport info structure (deprecated).
 *
 * @param client the JACK client structure.
 * @param tinfo current transport info structure.  The "valid" field
 * describes which fields contain valid data.
 *
 * @deprecated This is for compatibility with the earlier transport
 * interface.  Use jack_transport_query(), instead.
 *
 * @pre Must be called from the process thread.
 */
void jack_get_transport_info (jack_client_t *client,
			      jack_transport_info_t *tinfo) JACK_OPTIONAL_WEAK_DEPRECATED_EXPORT;

/**
 * Set the transport info structure (deprecated).
 *
 * @deprecated This function still exists for compatibility with the
 * earlier transport interface, but it does nothing.  Instead, define
 * a ::JackTimebaseCallback.
 */
void jack_set_transport_info (jack_client_t *client,
			      jack_transport_info_t *tinfo) JACK_OPTIONAL_WEAK_DEPRECATED_EXPORT;

#ifdef __cplusplus
}
#endif

#endif /* JACK_TRANSPORT_H */
