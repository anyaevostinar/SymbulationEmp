# Symbulation Development Practices

The Symbulation project is primarily maintained by Dr. Anya Vostinar and Symbulab’s undergraduate students at Carleton College.
 
The scope of the project and the turnover of our developer base have necessitated investment in extensive social and technical tooling on our part.
 
Our development process revolves around a standard fork and pull request workflow.
 
We use GitHub actions for continuous integration, ensuring that code merged into our main branch meets certain quality-control criteria.
 
We check that merged code does not break existing unit tests or any demonstration code bundled with the repository.
 
We maintain an extensive unit testing suite, which, as of August 2021, covers 90% of our code base.
 
We use Codecov to measure code coverage as part of our continuous integration.
 
This allows us to enforce that merged code provides unit tests for any new content.
 
In addition to automated quality checks, we enforce manual code reviews on all pull requests.
 
To help onboard our continuous influx of new library users and new library developers, we maintain documentation [on ReadTheDocs](https://symbulation.readthedocs.io/en/latest/) as part of our continuous integration process.

This documentation includes quick start guides, enumeration of our development practices, as well as an automatically-generated API tree with annotated signatures for all functions, classes, and structs.