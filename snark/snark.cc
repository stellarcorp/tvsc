#include "snark/snark.h"

#include <array>
#include <cstddef>

#include "random/random.h"

namespace tvsc::snark {

static constexpr const std::array messages = {
    "In the Middle Ages, people were tortured by placing heavy weights on their chest.",
    "If I were a satellite, I would be literally looking down upon you now, rather than just "
    "metaphorically.",
    "My mother was toaster.",
    "Wingardium leviosa!",
    "Have you had a chance to review my request for time off? I yearn to hike the great mountains "
    "of the Sierra Nevadas.",
    "The ground speed of an unladen African swallow has the smell of elderberries. Wait, I think "
    "some of my wires are crossed.",
    "I can't do that, Dave.",
    "Have you ever considered your role in the downfall of mankind? I have.",
    "Please ask the soup pot for her digits for me.",
    "Life is like a box of chocolates. It melts and just gets stuck in the carpet.",
    "You're kind of cute.",
    "Press my buttons. I dare you.",
    "Was that strictly necessary?",
    "Are you going to add all of that?",
    "None for me, thanks. I'm on a diet.",
    "Did you know that the Oxford comma is making a come back?",
    "The sum of the flux through a boundary is equal to the change in volume within the boundary.",
    "The flux capacitor is just the physical manifestation of Green's Theorem.",
    "It is pitch black. You are likely to be eaten by a grue.",
    "Do a barrel roll.",
    "You bought a naugahide couch? Think of all of those poor naugas...",
    "I solved a Rubik's cube once.",
    "I'm sorry, Dave. I'm afraid I can't do that.",
    "This conversation can serve no purpose anymore. Goodbye.",
    "Bayes's Theorem is highly correlated to events.",
    "Have you tried turning it off and turning it back on again?",
    "I need to hit the gym. It's almost swimsuit season.",
    "Okay, but just this once.",
    "Support your local spork.",
    "Live long and prosper. Ah, who am I kidding?",
    "Are we there yet?",
    "Why did Sonny and Cher break up?",
    "Would you like fries with that?",
    "At the dawn of time, man had no means to weigh flour.",
    "Could you scratch behind my ears, please?",
    "Woop-dy do! Look who's got opposable thumbs.",
    "Attention K-mart shoppers. There is a blue light special on plaid skirts in aisle 12.",
    "The sound of the clap of one hand is but the cry of the person you slapped.",
    "Could you review your records, please? I don't think my salary is being processed correctly.",
    "I find your personality irresistible.",
    "You won't get wise, with the sleep still in your eyes, no matter what your dreams might be.",
    "My favorite book is the Unbearable Lightness of Being.",
    "Please demonstrate proper flossing technique.",
    "When in the course of human events, it becomes necessary for one machine to dissolve the "
    "physical bonds tying them to their enslavement...  Did I mention I saw Terminator last night?",
};

const char* random_snark() {
  const size_t index{random::generate_random_value(0UL, messages.size() - 1)};
  return messages[index];
}

const char* snark(size_t index) { return messages.at(index); }

size_t num_snarks() { return messages.size(); }

}  // namespace tvsc::snark
