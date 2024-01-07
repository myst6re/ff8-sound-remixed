#pragma once

#include <QtCore>

class FF8Installation
{
public:
    FF8Installation();
private:
    enum RegLocation {
        LocationUser,
        LocationMachine
    };
    enum RegTarget {
        TargetNone,
        Target32,
        Target64
    };
    static QString regValue(const QString &regPath, const QString &regKey,
                            RegTarget target, RegLocation loc);
    static QString regValue(const QString &regPath, const QString &regKey,
                            RegLocation loc = LocationMachine);
    static inline QString regValuePath(const QString &regPath, const QString &regKey, RegLocation loc = LocationMachine) {
        return QDir::fromNativeSeparators(QDir::cleanPath(regValue(regPath, regKey, loc)));
    }
};
