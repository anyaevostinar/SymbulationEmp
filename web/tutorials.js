import getSettingsClickPromise from "./utilities/getSettingsClickPromise"
import getToggleClickPromise from "./utilities/getToggleClickPromise"
import getUpdatesThresholdPromise from "./utilities/getUpdatesThresholdPromise"
import getResetClickPromise from "./utilities/getResetClickPromise"
import getSynergyCheckPromise from "./utilities/getSynergyCheckPromise"

// advanceOn is a promise that, when resolved, will advance the tutorial to the next step.
// https://zhixingj.github.io/SymbulationEmp/web/symbulation.html
// ^^ that's the old tutorial with info to make steps out of.

/*
  This is where the tutorial objects are stored. 
  Tutorials have...
    -an id (in case we use tutorials.map to display them) 
    -a name that is displayed when the copilot menu is first opened
    -an array of steps
  Steps have...
    -a titleText string that is displayed at the top of the step
    -a bodyText string that is displayed below the title
    -a buttonLayout string. This is how CopilotWindow.js knows which component to use to display the step. 
      -see ./components for the different components
    -a buttonLabels array of strings. This is the text that is displayed on the buttons
    -an imgSrc string. This is the path to the image that is displayed in components that use images
    -an advanceOn promise. This is a promise that, when resolved, will advance the tutorial to the next step
      -see ./utilities for the different ways to create promises. They should be imported at the top of this file
    -an onAdvance function. This is a function that is called after onAdvance but before curStep is actually incremented. It is optional.
*/
//TODO: update above because advanceOn is now a function that returns a promise, not a promise itself
export const tutorials = [
  {
    id: 0,
    name: 'Placeholder',
    steps: [
      {
        titleText: 'Lorem Ipsum',
        bodyText: 'Select a tutorial or lab.',
        buttonLayout: 'oneContained',
        buttonLabels: ['Start Tutorial'],
      },
      {
        titleText: 'Welcome!',
        bodyText: 'Symbulation is a platform for simulating the evolution of symbionts and hosts.',
        imgSrc: './assets/sym-img-0.png',
        buttonLabels: ['Back', 'Next'],
        buttonLayout: 'twoContained',
      },
      {
        titleText: 'Endosymbiosis',
        bodyText: 'The symbionts are endosymbionts, meaning they live and reproduce inside of a host. Hosts collect resources and invest them into either defense against their symbionts (antagonism) or donation to their symbionts (mutualism).',
        imgSrc: './assets/sym-img-1.png',
        buttonLabels: ['Back', 'Next'],
        buttonLayout: 'twoContained',
      },
      {
        titleText: 'Symbionts',
        bodyText: 'Symbionts can then either steal resources from their host (antagonism) or donate resources back to their host (mutualism).',
        imgSrc: './assets/sym-img-2-recropped.png',
        buttonLabels: ['Back', 'Next'],
        buttonLayout: 'twoContained',
      },
      {
        titleText: 'Reproduction',
        bodyText: 'The more resources an organism has, the more it can reproduce. Organisms pass on just one gene: the interaction value. It ranges from -1 (most antagonistic) to 1 (most mutualistic).',
        imgSrc: './assets/sym-img-3.png',
        buttonLabels: ['Back', 'Next'],
        buttonLayout: 'twoContained',
      },
      {
        titleText: 'Symulation Setup',
        bodyText: 'Let\'s try a simulation! First, click \"Settings\" tab to open the settings.',
        buttonLayout: 'none',
        advanceOn: () => new Promise((resolve, reject) => {
          getSettingsClickPromise().then(resolve).catch(reject);
        })
      },
      {
        titleText: 'Synergy',
        bodyText: 'Take a look at \"Synergy\" under the global settings. When a Symbiont donates resources back to its host, those resources are multiplied by the synergy value. Try setting it to 10 and click \"Start\" to start the simulation.',
        imgSrc: './assets/sym-img-4.png',
        buttonLayout: 'imgOnly',
        advanceOn: () => new Promise((resolve, reject) => {
          getToggleClickPromise().then(resolve).catch(reject);
        })
      },
      {
        titleText: 'Simulating...',
        bodyText: 'Watch the simulation and see what happens. You can pause and resume the simulation at any time.',
        buttonLayout: 'none',
        advanceOn: () => new Promise((resolve, reject) => {
          getUpdatesThresholdPromise(150).then(resolve).catch(reject);
        }), 
        onAdvance: () => { 
          document.getElementById('toggle').click() // stop the simulation
          document.getElementById('emp__517_card_header').click() // close the settings so the user can see the antagonism to mutualism spectrum
        } 
      },
      {
        titleText: 'Results',
        bodyText: 'Look at the antagonism to mutualism color spectrum. It looks like the synergy value caused lots of mutualism!',
        buttonLayout: 'oneContained',
        buttonLabels: ['Next'],
      },
      {
        titleText: 'New Simulation',
        bodyText: 'Let\'s try another simulation. Press the \"Reset\" button.',
        buttonLayout: 'none',
        advanceOn: () => new Promise((resolve, reject) => {
          getResetClickPromise().then(resolve).catch(reject);
        })
        
      },
      {
        titleText: 'Antagonism',
        bodyText: 'Open the settings again and try lowering the synergy value down to 0.',
        buttonLayout: 'none',
        //advanceOn: getSynergyCheckPromise(0)
        advanceOn: () => new Promise((resolve, reject) => {
          getSynergyCheckPromise(0).then(resolve).catch(reject);
        })
      },
      {
        titleText: "Start New Simulation",
        bodyText: "Now, press start and watch the simulation.",
        buttonLayout: 'none',
        //advanceOn: getToggleClickPromise()
        advanceOn: () => new Promise((resolve, reject) => {
          getToggleClickPromise().then(resolve).catch(reject);
        })
      },
      // TODO: fix the jump bug that happens here
      {
        titleText: 'Simulating...',
        bodyText: 'Watch the simulation and see what happens. You can pause and resume the simulation at any time.',
        buttonLayout: 'none',
        //advanceOn: getUpdatesThresholdPromise(150),
        advanceOn: () => new Promise((resolve, reject) => {
          getUpdatesThresholdPromise(150).then(resolve).catch(reject);
        }),
        onAdvance: () => { 
          document.getElementById('toggle').click() // stop the simulation
          document.getElementById('emp__517_card_header').click() // close the settings so the user can see the antagonism to mutualism spectrum
        } 
      },
      {
        titleText: 'Results',
        bodyText: 'Look at the antagonism to mutualism color spectrum. It looks like the synergy value caused lots of antagonism!',
        buttonLayout: 'oneContained',
        buttonLabels: ['Next'],
      },
      {
        titleText: 'End',
        bodyText: 'End of tutorial',
        buttonLayout: 'none',
      }
    ]
  }
]