# Copilot

Copilot is an add-on to Symbulation's Web GUI used to make tutorials and other guidance using React and JavaScript.

## Table of Contents

[Overview](#overview)

[State Management](#State-Management)

[How to Make Tutorials](#How-to-Make-Tutorials)

[Pre-made Components](#Pre-made-Components)

[Utility Functions](#Utility-Functions)

## Overview

Copilot lives in the `web` folder. The top-level component is Copilot (`Copilot.js`). It renders a ThemeProvider div that lets its children access the [Material UI](https://mui.com/material-ui/) theme. It also imports Copilot's [Sass](https://sass-lang.com) stylesheet.

Copilot displays a question mark icon when Copilot is closed, or the Copilot window when it's opened. The Copilot window fetches the tutorial content, renders it, and handles state using a [Jotai](https://jotai.org) atom. It gets the tutorial content from `tutorials.js`, which contains arrays of JavaScript objects that make up the steps of  the tutorials. The Copilot window grabs the attributes of the current tutorial step and decides which tutorial component to render based on the step's `component` attribute. Once it knows which component to render, it passes all the attributes of the current step as props into that component.

Most tutorial components have buttons that let the user advance to the next step of the tutorial by updating the `tutorialTrackerAtom` atom. When the Copilot window sees that `tutorialTrackerAtom` has changed, it'll render the next step. Some tutorial components don't have buttons to advance the tutorial. Instead, the step has an `advanceOn` attribute. See the [How to Make Tutorials](#How-to-Make-Tutorials) section for a full explanation, but, to put it briefly, once `advanceOn` happens, the Copilot window will automatically advance to the next step.

"Utilities" are functions stored in the `utilities` folder. They are asynchronous functions used to interface between Copilot and Empirical and are designed to be used as `advanceOn`s.

Developers can create new tutorials in the `tutorials.js` file, create new components in the `components` folder, and new utilities in the `utilities` folder.

## State Management

Copilot uses [Jotai](https://jotai.org) for state management. There is one atom called `tutorialTrackerAtom` stored in the `atoms.js` file. It's an object with two key-value pairs:

**1:** `currentTutorialId` is the index of the tutorial that is currently being used in the parent `tutorials` array in `tutorials.js`. It's 0 by default and is currently not ever changed, but when new tutorials are made, this value can be changed.

**2:** `step` is the index of the step the user is currently on in the current tutorial. It's accessed by both the Copilot window and tutorial components that have buttons for going back and forth between steps.

## How to Make Tutorials

Tutorials are stored in the `tutorials.js` file. It exports one gigantic array called `tutorials`. Tutorials have 3 parts:

**1:** `id` is a unique integer to identify the tutorial. It's not currently used but exists in case more tutorials are added; it'd make it easy to [map](https://react.dev/learn/rendering-lists#where-to-get-your-key) through and display all the tutorials with React.

**2:** `name` is a string with a name for the tutorial.

**3:** `steps` is an array of objects that make up the steps of the tutorial. The Copilot window will load and display these.



Steps, the objects that make up the `steps` array, can have up to 7 parts. 

**1:** `titleText` is the large title that will be displayed at the top of the tutorial component. It's used on all of the pre-made components.

**2:** `bodyText` is smaller text displayed in the body of the tutorial component. It is also used on all of the pre-made components.

**3:** `imgSrc` is a string with the path to the image that will be displayed. Tutorial images are stored in the `web/assets` folder. Not all components use images, so this part is optional.

**4:** `buttonLabels` is an array of 1 or more strings. These strings will be displayed on the buttons in the corner of components that use buttons. Typically they'll be something like "Next" or "Back." Some components have two buttons; some have one; and some have none. So, button labels are optional too.

**5:** `advanceOn` is an asynchronous function. If a step has an `advanceOn`, the Copilot window will run it as soon as the user is on that step. It returns a promise, and once that promise is resolved, Copilot will automatically advance to the next step. `advanceOn` is made to be used with utility functions. The format used is as follows:

```javascript
advanceOn: () => new Promise((resolve, reject) => {
  getACoolPromise().then(resolve).catch(reject);
})
```

Whenever the promise that the utility function `getACoolPromise()` resolves, Copilot will advance to the next step. See the section on utility functions for more information on how these functions work. `advanceOn` is optional.

**6:** `onAdvance` is a function that will automatically run once the user advances past the tutorial step. It's also optional.

**7:** `component` is a React function component. The Copilot window will render whatever the current step's `component` is, and pass everything else about the step into it as props. There are several pre-made components: one with an image and two buttons, one with text and one button, etc. See the [section on pre-made components](#Pre-made-components). Instead of those pre-made components, new ones can be made as long as they're imported at the top of `tutorials.js`.

Here's an example of a step:

```javascript
{
  titleText: 'I am the titleText!',
  bodyText: 'I am the bodyText. Lorem ipsum dolor sit amet.',
  imgSrc: './assets/tut-img.png',
  buttonLabels: ['Label', '2nd Label'],
  component: TwoContained,
},
```

And here's how it looks:

<img src="https://p.ipic.vip/v6jwyb.png" alt="Screenshot 2024-03-25 at 7.09.09 AM" style="zoom:35%;" />

## Pre-made Components

Any valid React function component can be set as a step's `component`, but there are several pre-made components that can cover most scenarios. They are named based on their button/image layout. So, "OneContained" means there is one ["contained" MUI button](https://mui.com/material-ui/react-button/#contained-button).

#### OneContained

`OneContained` displays a title and one left-justified button right below the body text. The button will advance to the next step.

Requirements: `titleText`, `bodyText`, and one string in `buttonLabels`

<img src="https://p.ipic.vip/s2ufkq.png" alt="Screenshot 2024-03-25 at 2.31.07 PM" style="zoom:33%;" />

#### TwoContainedImage

`TwoContainedImage` displays a title, body text, one image, and two buttons in the bottom left and bottom right corners. The first button (bottom left) retreats back a step, and the second button (bottom right) advances to the next step.

Requirements: `titleText`, `bodyText`, `imgSrc`, and two strings in `buttonLabels`

<img src="https://p.ipic.vip/fm7029.png" alt="Screenshot 2024-03-25 at 2.31.10 PM" style="zoom:33%;" />

#### ImgOnly

`ImgOnly` displays a title, body text, and one image.

Requirements: `titleText`, `bodyText`, and `imgSrc`. It's also recommended to use an `advanceOn` since there are no buttons to advance to the next step.

<img src="https://p.ipic.vip/t76mu2.png" alt="Screenshot 2024-03-25 at 2.31.14 PM" style="zoom:33%;" />

#### None

`None` just displays a title and body text.

Requirements: `titleText` and `bodyText`. Using an `advanceOn` is recommended.

<img src="https://p.ipic.vip/u8ixdm.png" alt="Screenshot 2024-03-25 at 2.36.06 PM" style="zoom:33%;" />

#### NoneInnerHTML

`NoneInnerHTML` displays a title and body text just like `None`, but instead of just displaying the body text, it will set the body's inner HTML to whatever is in `bodyText`. This is useful for displaying links with anchor tags, using spans, etc.

Requirements: `titleText` and `bodyText`. Using an `advanceOn` is recommended.

<img src="https://p.ipic.vip/nv2mza.png" alt="Screenshot 2024-03-25 at 2.40.45 PM" style="zoom:33%;" />



## Utility Functions

Utility functions live in the `web/utilities` folder. These are asynchronous functions that return a promise that resolves when something in Symbulation happens, hence the "getPromise()" naming scheme. These are useful because they allow the Copilot window to react to what's happening outside of Copilot in Symbulation via its DOM elements. They're intended to be used as the `advanceOn` for tutorial steps. 

The utility functions are all on a 200ms `setTimeout` delay. This means that for the first 200ms after the function is called (i.e., first thing when the Copilot window loads the step), the promise that they return won't do anything. The reason for this delay is because Symbulation's DOM elements often update *slightly* later than Copilot, meaning there is a brief period of time where Copilot's utility functions get either nothing or outdated information from Symbulation's DOM. This delay is unnoticeable unless a user were to both begin a step and interact with Symbulation in less than 200ms.

Below is a brief description of each of the utility functions.

**getToggleClickPromise**

`getResetClickPromise()` returns a promise that resolves when Symbulation's toggle button (that displays either "Start" or Pause") is clicked. 

**getSettingsClickPromise**

`getSettingsClickPromise()` returns a promise that resolves when Symbulation's settings card header is clicked. Note that this function works regardless of if the settings tab is open, so it can be used to advance either when the user opens the settings tab or closes it.

**getSynergyCheckPromise**

`getSynergyCheckPromise()` accepts one argument, `goalValue`, and returns a promise that resolves once the synergy value in Symbulation's settings has been set to the `goalValue`.

**getResetClickPromise**

`getResetClickPromise()` returns a promise that resolves when Symbulation's "Reset" button has been clicked.

**getUpdatesThresholdPromise**

`getUpdatesThresholdPromise()` accepts one argument, `threshold` and returns a promise that resolves once the number of updates in the current Symbulation simulation is greater than or equal to the `threshold` value.
