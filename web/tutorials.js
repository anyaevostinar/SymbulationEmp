import getSettingsClickPromise from "./utilities/getSettingsClickPromise"
import getToggleClickPromise from "./utilities/getToggleClickPromise"
import getUpdatesThresholdPromise from "./utilities/getUpdatesThresholdPromise"
import getResetClickPromise from "./utilities/getResetClickPromise"
import getSynergyCheckPromise from "./utilities/getSynergyCheckPromise"

import OneContained from "./components/OneContained"
import TwoContainedImage from "./components/TwoContainedImage"
import ImgOnly from "./components/ImgOnly"
import NoneInnerHTML from "./components/NoneInnerHTML"
import None from "./components/None"



export const tutorials = [
  {
    id: 0,
    name: 'MainTutorial',
    steps: [
      {
        titleText: 'Tutorial',
        bodyText: 'Need help getting started? Try this tutorial!',
        buttonLabels: ['Start Tutorial'],
        component: OneContained,
      },/*
      {
        titleText: 'I am the titleText!',
        bodyText: 'I am the bodyText. Lorem ipsum dolor sit amet.',
        imgSrc: './assets/tut-img.png',
        buttonLabels: ['Label', '2nd Label'],
        component: TwoContained,
      },*/
      {
        titleText: 'I\'m the OneContained component',
        bodyText: 'I have one button. It displays the first (and only) button label.',
        buttonLabels: ['Onward!'],
        component: OneContained,
      },
      {
        titleText: 'I\'m the TwoContainedImage component',
        bodyText: 'I have two buttons. The first button displays the first button label, and the second button displays the second button label. I also have an image.',
        imgSrc: './assets/tut-img.png',
        buttonLabels: ['Back', 'Next'],
        component: TwoContainedImage,
      },/*
      {
        titleText: 'I\'m the ImgOnly component',
        bodyText: 'No buttons no problem. I just display an image and body text. You\'ll probably want to use an advanceOn function with me.',
        imgSrc: './assets/tut-img.png',
        component: ImgOnly,
      },*//*
      {
        titleText: 'I\'m the None component',
        bodyText: 'I don\'t have any buttons or images. I\'m just here to display text. You\'ll probably want to use an advanceOn function with me too.',
        component: None,
      },*/
      {
        titleText: 'I\'m the NoneInnerHTML component.',
        bodyText: 'I display the bodyText as innerHTML using React\'s dangerouslySetInnerHTML prop—I like to live on the edge. I can do things like <a href="https://www.google.com">anchor tags</a>.',
        component: NoneInnerHTML,
      },
      {
        titleText: 'Welcome!',
        bodyText: 'Symbulation is a platform for simulating the evolution of symbionts and hosts.',
        imgSrc: './assets/sym-img-0.png',
        buttonLabels: ['Back', 'Next'],
        component: TwoContainedImage,
      },
      {
        titleText: 'Endosymbiosis',
        bodyText: 'The symbionts are endosymbionts, meaning they live and reproduce inside of a host. Hosts collect resources and invest them into either defense (antagonism) or donation (mutualism) to their symbionts.',
        imgSrc: './assets/sym-img-1.png',
        buttonLabels: ['Back', 'Next'],
        component: TwoContainedImage,
      },
      {
        titleText: 'Symbionts',
        bodyText: 'Symbionts can then either steal resources from their host (antagonism) or donate resources back to their host (mutualism).',
        imgSrc: './assets/sym-img-2-recropped.png',
        buttonLabels: ['Back', 'Next'],
        component: TwoContainedImage,
      },
      {
        titleText: 'Reproduction',
        bodyText: 'The faster an organism collects resources, the faster it can reproduce. Organisms pass on just one gene: the interaction value. It ranges from -1 (most antagonistic) to 1 (most mutualistic).',
        imgSrc: './assets/sym-img-3.png',
        buttonLabels: ['Back', 'Next'],
        component: TwoContainedImage,
      },
      {
        titleText: 'Symulation Setup',
        bodyText: 'Let\'s try a simulation! First, click \"Settings\" tab to open the settings.',
        component: None,
        advanceOn: () => new Promise((resolve, reject) => {
          getSettingsClickPromise().then(resolve).catch(reject);
        })
      },
      {
        titleText: 'Synergy',
        bodyText: 'Take a look at \"Synergy\" under the global settings. When a Symbiont donates resources back to its host, those resources are multiplied by the synergy value. Try setting it to 10 and click \"Start\" to start the simulation.',
        imgSrc: './assets/sym-img-4.png',
        component: ImgOnly,
        advanceOn: () => new Promise((resolve, reject) => {
          getToggleClickPromise().then(resolve).catch(reject);
        })
      },
      {
        titleText: 'Simulating...',
        bodyText: 'Watch the simulation and see what happens. You can pause and resume the simulation at any time.',
        component: None,
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
        buttonLabels: ['Next'],
        component: OneContained,
      },
      {
        titleText: 'New Simulation',
        bodyText: 'Let\'s try another simulation. Press the \"Reset\" button.',
        advanceOn: () => new Promise((resolve, reject) => {
          getResetClickPromise().then(resolve).catch(reject);
        }),
        component: None,
      },
      {
        titleText: 'Antagonism',
        bodyText: 'Open the settings again and try lowering the synergy value down to 0.',
        //advanceOn: getSynergyCheckPromise(0)
        advanceOn: () => new Promise((resolve, reject) => {
          getSynergyCheckPromise(0).then(resolve).catch(reject);
        }),
        component: None,
      },
      {
        titleText: "Start New Simulation",
        bodyText: "Now, press start and watch the simulation.",
        //advanceOn: getToggleClickPromise()
        advanceOn: () => new Promise((resolve, reject) => {
          getToggleClickPromise().then(resolve).catch(reject);
        }),
        component: None,
      },
      {
        titleText: 'Simulating...',
        bodyText: 'Watch the simulation and see what happens. You can pause and resume the simulation at any time.',
        //advanceOn: getUpdatesThresholdPromise(150),
        advanceOn: () => new Promise((resolve, reject) => {
          getUpdatesThresholdPromise(150).then(resolve).catch(reject);
        }),
        onAdvance: () => { 
          document.getElementById('toggle').click() // stop the simulation
          document.getElementById('emp__517_card_header').click() // close the settings so the user can see the antagonism to mutualism spectrum
        },
        component: None,
      },
      {
        titleText: 'Results',
        bodyText: 'Look at the antagonism to mutualism color spectrum. It looks like the synergy value caused lots of antagonism!',
        buttonLabels: ['Next'],
        component: OneContained,
      },
      {
        titleText: 'End',
        bodyText: 'That\'s all for this tutorial! For more information, check out the <a href=https://symbulation.readthedocs.io/en/latest/>documentation</a>.',
        component: NoneInnerHTML,
      },
    ]
  }
]