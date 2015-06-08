
ABOUT
=====

Telepathy-Qt-Generator is a tool to generate TelepathyQt Services source
code from Telepathy (XML) specification.

Most of the interfaces can be fully (if there is no custom logic) or
partially generated from telepathy specification.

For example, BaseConnectionAliasingInterface code is generated for 100%,
BaseChannelSASLAuthenticationInterface code have just two extra lines,
added to generation result.

USAGE
=====

    telepathy-qt-generator <specs file>

Output consists of three sections: public header, private (internal) header and an implementation code.

WEBSITE AND REPOSITORY
======================

The project is hosted at:

https://github.com/TelepathyQt/telepathy-qt-generator
