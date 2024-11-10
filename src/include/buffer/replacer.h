//===----------------------------------------------------------------------===//
//
// replacer.h
// I acknowledge that I use CMU 15-445 BusTub as a reference.
//===----------------------------------------------------------------------===//

#include "utils/config.h"

/**
 * Replacer is an abstract class that tracks the replacement policy of a buffer.
 */
class Replacer {
  public:
    Replacer() = default;
    virtual ~Replacer() = default;

    /**
     * Remove the victim frame from the buffer pool by the replacement policy.
     * @param[out] frame_id id of the frame that was removed, nullptr if no 
     * victim frame was found.
     * @return true if a victim frame is found, false otherwise.
     */
    virtual auto Victim(frame_id_t *frame_id) -> bool = 0;
    
    /**
     * Pins the frame with the given frame_id, indicating that it should not be 
     * victimized until it is unpinned.
     * @param frame_id the id of the frame to pin.
     */
    virtual auto Pin(frame_id_t frame_id) = 0;

    /**
     * Unpins the frame with the given frame_id, indicating that it can now be
     * victimized.
     * @param frame_id the id of the frame to unpin.
     */
    virtual auto Unpin(frame_id_t frame_id) = 0;

    /**
     * @return the number of elements in the replacer that can be victimized.
     */
    virtual auto Size() -> size_t = 0;
};