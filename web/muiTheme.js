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
    },
    // Add more colors here
  },
});

export default muiTheme;