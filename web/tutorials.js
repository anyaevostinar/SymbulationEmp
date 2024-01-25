import getToggleClickPromise from "./utilities/getToggleClickPromise"
// advanceOn is a promise that, when resolved, will advance the tutorial to the next step.
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