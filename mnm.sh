#!/bin/bash
make web
sass ./web/copilot.scss ./web/copilot.css
npx webpack
make serve