# Copilot

Copilot is an add-on to Symbulation's Web GUI used to make tutorials and other guidance using React and JavaScript.

## Overview

Copilot lives in the `web` folder. The top-level component is `Copilot.js`; it renders a `ThemeProvider` div that lets its children access the [Material UI](https://mui.com/material-ui/) theme stored in `muiTheme.js`. It also imports Copilot's [Sass](https://sass-lang.com) stylesheet: `copilot.scss`.

`Copilot.js` displays a question mark icon (`OpenButton.js`) when Copilot is closed, or the Copilot window (`CopilotWindow.js`), when it's opened.



Todo:

-context with jotai

-explanation of copilot window

-explanation of tutorials.js steps

-explain all the different promise utilities

-exaplanation of the different premade components

​    -with images

-assets folder and aspect ratios?