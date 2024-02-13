import getToggleClickPromise from "./utilities/getToggleClickPromise"
// advanceOn is a promise that, when resolved, will advance the tutorial to the next step.
// https://zhixingj.github.io/SymbulationEmp/web/symbulation.html
// ^^ that's the old tutorial with info to make steps out of.
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
        bodyText: 'The symbionts are endosymbionts, meaning they live and reproduce inside of a host. Hosts can either collect resources and invest them into defense against their symbionts or donation to their symbionts.',
        imgSrc: './assets/sym-img-1.png',
        buttonLabels: ['Back', 'Next'],
        buttonLayout: 'twoContained',
      },
      {
        titleText: 'Symbionts',
        bodyText: 'Symbionts can then either steal resources from their host or donate resources back.',
        imgSrc: './assets/sym-img-2-recropped.png',
        // todo: do a less cropped version of this image
        buttonLabels: ['Back', 'Next'],
        buttonLayout: 'twoContained',
      },
      {
        titleText: 'Welcome!',
        bodyText: 'Try clicking the "Start" button.',
        buttonLayout: 'none',
        advanceOn: getToggleClickPromise()
      },
      {
        titleText: 'You did it!',
        bodyText: 'You clicked the "Start" button.',
        buttonLayout: 'none',
      }
    ]
  }
]