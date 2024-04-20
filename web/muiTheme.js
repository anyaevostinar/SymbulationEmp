/**
 * @name muiTheme
 * @description Material-UI theme for Copilot
 * @returns {object} a Material-UI theme object
 * This does not affect anything outside of Copilot, but it can be used to change the color of the buttons in Copilot
 */
import { createTheme } from '@mui/material/styles';

const muiTheme = createTheme({
  palette: {
    background: {
      default: '#fff',
    },
    text: {
      primary: '#212529',
    },
    primary: {
      main: '#d3a1d6',
    }
  },
});

export default muiTheme;