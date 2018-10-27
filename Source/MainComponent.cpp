#include "MainComponent.h"

MainComponent::MainComponent()
{
    addAndMakeVisible(envEd);
    setSize(600, 400);
}

void MainComponent::resized()
{
    envEd.setBounds(getLocalBounds());
}
