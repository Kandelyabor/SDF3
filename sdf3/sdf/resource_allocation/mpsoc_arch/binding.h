/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   binding.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   April 3, 2006
 *
 *  Function        :   Binding of SDF components to architecture components.
 *
 *  History         :
 *      03-04-06    :   Initial version.
 *
 * $Id: binding.h,v 1.1.1.1.2.1 2010-04-25 00:10:19 mgeilen Exp $
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * In other words, you are welcome to use, share and improve this program.
 * You are forbidden to forbid anyone else to use, share and improve
 * what you give them.   Happy coding!
 */

#ifndef SDF_RESOURCE_ALLOCATION_MPSOC_BINDING_H_INCLUDED
#define SDF_RESOURCE_ALLOCATION_MPSOC_BINDING_H_INCLUDED

#include "sdf/base/timed/graph.h" // IWYU pragma: keep
#include <memory>

namespace SDF {
class ComponentBinding {
public:
    // Constructor
    ComponentBinding(const SDFcomponent *c, uint nrParams) : component(c), params(nrParams){};

    // Rule of five: delete copy/move constructors and assignment operators
    ComponentBinding(const ComponentBinding&) = delete;
    ComponentBinding& operator=(const ComponentBinding&) = delete;
    ComponentBinding(ComponentBinding&&) = delete;
    ComponentBinding& operator=(ComponentBinding&&) = delete;

    // Destructor
    ~ComponentBinding() = default;

    // Component
    [[nodiscard]] const SDFcomponent *getComponent() const { return component; };

    // Parameters
    [[nodiscard]] double getValue(uint param) const;
    void setValue(uint param, double val) { params[param] = val; };
    [[nodiscard]] uint nrParams() const { return static_cast<uint>(params.size()); };

private:
    // Component
    const SDFcomponent *component;

    // Parameters
    std::vector<double> params;
};

using ComponentBindings = std::list<std::unique_ptr<ComponentBinding>>;
using ComponentBindingRefs = std::list<ComponentBinding *>;
using ComponentBindingsIter = ComponentBindings::iterator;
using ComponentBindingsCIter = ComponentBindings::const_iterator;

/**
 * CompBindings
 * Container for bindings of components to resource
 */
class CompBindings {
public:
    enum class Type { TypeSum = 0, TypeMax };

    // Constructor
    explicit CompBindings(Type t = Type::TypeSum);
    explicit CompBindings(std::vector<double> &maxValues, Type t = Type::TypeSum);

    // Rule of five: delete copy/move constructors and assignment operators
    CompBindings(const CompBindings&) = delete;
    CompBindings& operator=(const CompBindings&) = delete;
    CompBindings(CompBindings&&) = delete;
    CompBindings& operator=(CompBindings&&) = delete;

    // Destructor
    ~CompBindings();

    // Type of binding function (sum or max)
    [[nodiscard]] Type getType() const { return type; };

    // Parameters
    [[nodiscard]] double getMaxVal(uint param) const;
    [[nodiscard]] double getCurVal(uint param) const;
    [[nodiscard]] uint nrParams() const { return static_cast<uint>(maxVals.size()); };

    // Bind component to resource
    bool bind(std::unique_ptr<ComponentBinding> b);

    // Remove binding from component to resource
    bool unbind(const SDFcomponent *c);

    // Find binding of component to resource
    ComponentBinding *find(const SDFcomponent *c);

    // List of all components bound to the resource
    ComponentBindingsIter begin() { return bindings.begin(); };
    ComponentBindingsIter end() { return bindings.end(); };
    [[nodiscard]] ComponentBindingsCIter begin() const { return bindings.begin(); };
    [[nodiscard]] ComponentBindingsCIter end() const { return bindings.end(); };

private:
    Type type;
    ComponentBindings bindings;
    std::vector<double> maxVals;
    std::vector<double> curVals;
};
} // namespace SDF
#endif
