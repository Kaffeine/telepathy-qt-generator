#include <QDebug>

#include <QFile>
#include <QDomDocument>
#include <QStringList>

#include "CInterfaceGenerator.hpp"

static const bool skipDeprecatedEntries = true;
static const QLatin1String s_deprecatedElement = QLatin1String("tp:deprecated");

enum SpecType {
    BaseClass = 4,
    InterfaceClass = 6
};

void processSpec(const QString &fileName)
{
    QFile xmlFile(fileName);
    xmlFile.open(QIODevice::ReadOnly);

    QDomDocument document;
    document.setContent(xmlFile.readAll());
    xmlFile.close();

    const QDomElement interfaceElement = document.documentElement().firstChildElement(QLatin1String("interface"));
    QString interfaceName = interfaceElement.attribute(QLatin1String("name"));
    if (!interfaceName.startsWith(QLatin1String("org.freedesktop.Telepathy."))) {
        qDebug() << "File doesn't contain telepathy spec in known format (Error 1)";
        return;
    }

    const QStringList interfaceNameParts = interfaceName.split(QLatin1Char('.'));

    const int partsOfName = interfaceNameParts.count();

    switch (partsOfName) {
    case BaseClass: // Generic interface
    case InterfaceClass: // Attached interface
        break;
    default:
        qDebug() << "File doesn't contain telepathy spec in known format (Error 2)";
        return;
    }

    CInterfaceGenerator::InterfaceSubType subType = CInterfaceGenerator::InterfaceSubTypeInvalid;

    if (partsOfName == BaseClass) {
        subType = CInterfaceGenerator::InterfaceSubTypeBaseClass;
    } else {
        if (interfaceNameParts.at(4) == QLatin1String("Interface")) {
            subType = CInterfaceGenerator::InterfaceSubTypeInterface;
        } else if (interfaceNameParts.at(4) == QLatin1String("Type")) {
            subType = CInterfaceGenerator::InterfaceSubTypeType;
        }
    }

    if (subType == CInterfaceGenerator::InterfaceSubTypeInvalid) {
        qDebug() << "Spec sub type is unknown. (Error 3)";
        return;
    }

    CInterfaceGenerator generator;
    generator.setFullName(interfaceName);
    generator.setType(interfaceNameParts.at(3));
    generator.setSubType(subType);

    generator.setNode(document.documentElement().attribute(QLatin1String("name")));

    const QDomElement annotationElement = interfaceElement.firstChildElement(QLatin1String("annotation"));
    if (!annotationElement.isNull()) {
        const QString annotationName = annotationElement.attribute(QLatin1String("name"));
        if (annotationName == QLatin1String("org.freedesktop.DBus.Property.EmitsChangedSignal")) {
            generator.setEmitPropertiesChangedSignal(true);
        }
    }

    QDomElement propertyElement = interfaceElement.firstChildElement(QLatin1String("property"));

    while (!propertyElement.isNull()) {
        if (skipDeprecatedEntries && propertyElement.firstChildElement(s_deprecatedElement).isNull()) {
            // Element is *not* deprecated.
            CInterfaceProperty *property = new CInterfaceProperty();
            property->setName(propertyElement.attribute(QLatin1String("name")));
            property->setTypeFromStr(propertyElement.attribute(QLatin1String("type")), propertyElement.attribute(QLatin1String("tp:type")));
            property->setImmutable(propertyElement.attribute(QLatin1String("tp:immutable")) == QLatin1String("yes"));

            QDomElement docString = propertyElement.firstChildElement(QLatin1String("tp:docstring"));
            if (!docString.isNull()) {
                QDomNodeList children = docString.childNodes();
                for (int i = 0; i < children.count(); ++i) {
                    if (children.at(i).toElement().text() == QLatin1String("This property cannot change during the lifetime of the channel.")) {
                        property->setUnchangeable(true);
                        break;
                    }
                }
            }
            generator.m_properties.append(property);
        }

        propertyElement = propertyElement.nextSiblingElement(QLatin1String("property"));
    }

    QDomElement methodElement = interfaceElement.firstChildElement(QLatin1String("method"));

    while (!methodElement.isNull()) {
        if (skipDeprecatedEntries && methodElement.firstChildElement(s_deprecatedElement).isNull()) {
            // Element is *not* deprecated.

            CInterfaceMethod *method = new CInterfaceMethod(methodElement.attribute(QLatin1String("name")));

            QDomElement argElement = methodElement.firstChildElement(QLatin1String("arg"));

            while (!argElement.isNull()) {
                CMethodArgument arg;
                arg.setName(argElement.attribute(QLatin1String("name")));
                arg.setTypeFromStr(argElement.attribute(QLatin1String("type")), argElement.attribute(QLatin1String("tp:type")));
                arg.setDirection(argElement.attribute(QLatin1String("direction")));

                method->arguments.append(arg);

                argElement = argElement.nextSiblingElement(QLatin1String("arg"));
            }

            generator.m_methods.append(method);
        }

        methodElement = methodElement.nextSiblingElement(QLatin1String("method"));
    }


    QDomElement signalElement = interfaceElement.firstChildElement(QLatin1String("signal"));

    while (!signalElement.isNull()) {
        if (skipDeprecatedEntries && signalElement.firstChildElement(s_deprecatedElement).isNull()) {
            // Element is *not* deprecated.

            CInterfaceSignal *signal = new CInterfaceSignal(signalElement.attribute(QLatin1String("name")));

            QDomElement argElement = signalElement.firstChildElement(QLatin1String("arg"));

            while (!argElement.isNull()) {
                CMethodArgument arg;
                arg.setName(argElement.attribute(QLatin1String("name")));
                arg.setTypeFromStr(argElement.attribute(QLatin1String("type")), argElement.attribute(QLatin1String("tp:type")));
                arg.setDirection(QLatin1String("in"));

                signal->arguments.append(arg);

                argElement = argElement.nextSiblingElement(QLatin1String("arg"));
            }

            generator.m_signals.append(signal);
        }

        signalElement = signalElement.nextSiblingElement(QLatin1String("signal"));
    }

    generator.prepare();

    printf("Generated code for %s spec\n\n", fileName.toLocal8Bit().constData());

    printf("--- Public header: ---\n");
    printf("%s", generator.generateHeaderInterface().toLocal8Bit().constData());
    printf("--- Private (internal) header: ---\n");
    printf("%s", generator.generateHeaderAdaptee().toLocal8Bit().constData());
    printf("--- Source file: ---\n");
    printf("%s", generator.generateImplementations().toLocal8Bit().constData());
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Usage: %s <specs file>\n", argv[0]);
        return 0;
    }

    processSpec(QString::fromLocal8Bit(argv[1]));

    return 0;
}
