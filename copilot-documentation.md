# Copilot

Copilot is an add-on to Symbulation's Web GUI used to make tutorials and other guidance using React and JavaScript.

## Table of Contents

[Overview](#overview)

[State Management](#State Management)

[How to Make Tutorials](#How to Make Tutorials)

## Overview

Copilot lives in the `web` folder. The top-level component is Copilot (`Copilot.js`). It renders a ThemeProvider div that lets its children access the [Material UI](https://mui.com/material-ui/) theme. It also imports Copilot's [Sass](https://sass-lang.com) stylesheet.

Copilot displays a question mark icon when Copilot is closed, or the Copilot window when it's opened. The Copilot window fetches the tutorial content, renders it, and handles state using a [Jotai](https://jotai.org) atom. It gets the tutorial content from `tutorials.js`, which contains arrays of JavaScript objects that make up the steps of  the tutorials. The Copilot window grabs the attributes of the current tutorial step and decides which tutorial component to render based on the step's `component` attribute. Once it knows which component to render, it passes all the attributes of the current step as props into that component.

Most tutorial components have buttons that let the user advance to the next step of the tutorial by updating the `tutorialTrackerAtom` atom. When the Copilot window sees that `tutorialTrackerAtom` has changed, it'll render the next step. Some tutorial components don't have buttons to advance the tutorial. Instead, the step has an `advanceOn` attribute. See the [How to Make Tutorials](#How to Make Tutorials) section for a full explanation, but, to put it briefly, once `advanceOn` happens, the Copilot window will automatically advance to the next step.

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

Steps, the objects that make up the `steps` array, can have up to 5 parts. 

**1:** `titleText` is

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

<img src="/Users/bennett/Desktop/Screenshot 2024-03-25 at 7.09.09 AM.png" alt="Screenshot 2024-03-25 at 7.09.09 AM" style="zoom:40%;" />





Todo:



-explanation of tutorials.js steps

-explain all the different promise utilities

-exaplanation of the different premade components

​    -with images

