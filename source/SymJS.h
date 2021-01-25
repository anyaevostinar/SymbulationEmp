#include "web/init.h"

// All JS code related to game
// Modifies the content of the challenge box according to the challenge user has reached
EM_JS(void, modifyChallenge, (const char* str, int ind, int challenge_number), {
  if (ind >= challenge_number) {
    $('#playGame .modal-title').html("Congrats!");
    $('#playGame .modal-body').text("There are no challenges remaining!");
  } else if (ind == 0){ // first challenge, no need to remind users of reset
    $('#playGame .modal-title').html("Challenge " + (ind + 1));
    $('#playGame .modal-body').html(UTF8ToString(str));
  } else {
    $('#playGame .modal-title').html("Challenge " + (ind + 1));
    $('#playGame .modal-body').html(UTF8ToString(str) + "<br><b>Click Reset to restart the simulation.</b>");
  }
});

// Toggles the challenge modal to show the challenge
EM_JS(void, showChallenge, (), {
  $('#playGame').modal('toggle');
});

// Toggles successAlert modal to show success of challenge
EM_JS(void, showSuccess, (), {
  $('#successAlert').modal('toggle')
});

// Toggles failureAlert modal to show failure of challenge
EM_JS(void, showFailure, (), {
  $('#failureAlert').modal('toggle')
});