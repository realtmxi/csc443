#include <list>
#include <vector>
#include <shared_mutex>
#include "replacer.h"
#include "common/config.h"

/**
 * ClockReplacer implements the clock replacement policy, which approximates the
 * LRU(Least Recently Used) policy.
 * Each page has a reference bit, when a page is accessed, set its bit to 1.
 * Organize all pages in a circular buffer with a "clock hand" that sweeps over
 * pages in order:
 * As the hand visits each page, check if its bit is set to 1.
 * If yes, set to zero. If no, then evict the page.
 */
class ClockReplacer: public Replacer {
	public:
		/**
		 * Create a new ClockReplacer.
		 * @param num_pages the maximum number of pages the ClockReplacer can hold
		 */
		ClockReplacer(size_t num_pages);

		~ClockReplacer() override;

		auto Victim(frame_id_t *frame_id) -> bool override;

		void Pin(frame_id_t frame_id) override;

		void Unpin(frame_id_t frame_id) override;

		auto Size() -> size_t override;

		private:
			frame_id_t clock_hand_ = 0;
			std::vector<std::tuple<bool, bool>> frames_;
			std::shared_mutex latch_;
};