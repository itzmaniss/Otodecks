    // Sliders otherwise sit at the bottom of their range, which would start the
    // deck silent and stopped. Set without notifying: the player already holds
    // these values, so there is nothing to push to it.
    volSlider.setValue(defaultGain, dontSendNotification);
    speedSlider.setValue(defaultSpeed, dontSendNotification);

    // Normal speed is the value the user returns to most, so put it in the
    // middle of the travel rather than a quarter of the way along.
    speedSlider.setSkewFactorFromMidPoint(defaultSpeed);
