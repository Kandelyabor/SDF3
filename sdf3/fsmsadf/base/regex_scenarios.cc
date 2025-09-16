/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   regex_scenarios.h
 *
 *  Author          :   Hadi Alizadeh Ara (s.h.seyyed.alizadeh@tue.nl)
 *
 *  Date            :   July 30, 2017
 *
 *  Function        :   defines a regular expression that contains scenario information
 *
 *  History         :
 *      30-07-17    :   Initial version.
 *
 * $Id:   $
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

#include "regex_scenarios.h"
#include <sstream>

std::unique_ptr<RegExPersistentTokens> BlankS::checkConsistency() const { return nullptr; }

std::unique_ptr<RegExPersistentTokens> PrimitiveS::checkConsistency() const {

    auto rt = std::make_unique<RegExPersistentTokens>();

    rt->persistentTokensOfExpression = this->persistentTokensOfScenario;
    rt->persistentTokensFinalOfExpression = this->persistentTokensFinalOfScenario;

    return rt;
}

std::unique_ptr<RegExPersistentTokens> SequenceS::checkConsistency() const {
    auto* firstExp = dynamic_cast<RegExScenarios*>((this->getFirst()).get());
    std::unique_ptr<RegExPersistentTokens> f = firstExp->checkConsistency();

    auto *secondExp = dynamic_cast<RegExScenarios *>((this->getSecond()).get());
    std::unique_ptr<RegExPersistentTokens> s = secondExp->checkConsistency();

    if (f != nullptr) {
        if (f->persistentTokensFinalOfExpression == s->persistentTokensOfExpression) {
            auto newRegExPersistentTokens = std::make_unique<RegExPersistentTokens>();
            newRegExPersistentTokens->persistentTokensFinalOfExpression =
                    s->persistentTokensFinalOfExpression;
            newRegExPersistentTokens->persistentTokensOfExpression =
                    f->persistentTokensOfExpression;

            return newRegExPersistentTokens;
        }

        std::stringstream exp;
        this->toString(exp);
        throw CException("The graph is not consistent for the set of scenario sequences formalized "
                         "by subexpression "
                         + CString(exp.str()));
    }
    auto newRegExPersistentTokens = std::make_unique<RegExPersistentTokens>();
    newRegExPersistentTokens->persistentTokensFinalOfExpression =
            s->persistentTokensFinalOfExpression;
    newRegExPersistentTokens->persistentTokensOfExpression = s->persistentTokensOfExpression;

    return newRegExPersistentTokens;
}

std::unique_ptr<RegExPersistentTokens> ChoiceS::checkConsistency() const {
    auto *leftExp = dynamic_cast<RegExScenarios *>((this->getLeft()).get());
    std::unique_ptr<RegExPersistentTokens> l = leftExp->checkConsistency();

    auto *rightExp = dynamic_cast<RegExScenarios *>((this->getRight()).get());
    std::unique_ptr<RegExPersistentTokens> r = rightExp->checkConsistency();

    if (l->persistentTokensOfExpression == r->persistentTokensOfExpression
        && l->persistentTokensFinalOfExpression == r->persistentTokensFinalOfExpression) {
        auto newRegExPersistentTokens = std::make_unique<RegExPersistentTokens>();
        newRegExPersistentTokens->persistentTokensFinalOfExpression =
                l->persistentTokensFinalOfExpression;
        newRegExPersistentTokens->persistentTokensOfExpression = l->persistentTokensOfExpression;

        return newRegExPersistentTokens;
    }
    std::stringstream exp;
    this->toString(exp);
    throw CException("The graph is not consistent for the set of scenario sequences formalized "
                     "by subexpression "
                     + CString(exp.str()));
}

std::unique_ptr<RegExPersistentTokens> KleeneIterationS::checkConsistency() const {
    auto *baseExp = dynamic_cast<RegExScenarios *>((this->getBase()).get());
    std::unique_ptr<RegExPersistentTokens> b = baseExp->checkConsistency();

    if (b->persistentTokensFinalOfExpression == b->persistentTokensOfExpression) {
        auto newRegExPersistentTokens = std::make_unique<RegExPersistentTokens>();
        newRegExPersistentTokens->persistentTokensFinalOfExpression =
                b->persistentTokensFinalOfExpression;
        newRegExPersistentTokens->persistentTokensOfExpression = b->persistentTokensOfExpression;

        return newRegExPersistentTokens;
    }
    std::stringstream exp;
    this->toString(exp);
    throw CException("The graph is not consistent for the set of scenario sequences formalized "
                     "by subexpression "
                     + CString(exp.str()));
}

std::unique_ptr<RegExPersistentTokens> ConstantIterationS::checkConsistency() const {
    auto *baseExp = dynamic_cast<RegExScenarios *>((this->getBase()).get());
    std::unique_ptr<RegExPersistentTokens> b = baseExp->checkConsistency();

    if (b->persistentTokensFinalOfExpression == b->persistentTokensOfExpression) {
        auto newRegExPersistentTokens = std::make_unique<RegExPersistentTokens>();
        newRegExPersistentTokens->persistentTokensFinalOfExpression =
                b->persistentTokensFinalOfExpression;
        newRegExPersistentTokens->persistentTokensOfExpression = b->persistentTokensOfExpression;

        return newRegExPersistentTokens;
    }
    std::stringstream exp;
    this->toString(exp);
    throw CException("The graph is not consistent for the set of scenario sequences formalized "
                     "by subexpression "
                     + CString(exp.str()));
}

std::unique_ptr<RegExPersistentTokens> OmegaSequenceS::checkConsistency() const {
    auto *withExp = dynamic_cast<RegExScenarios *>((this->getWithOmega()).get());
    std::unique_ptr<RegExPersistentTokens> w = withExp->checkConsistency();

    if (w->persistentTokensFinalOfExpression != w->persistentTokensOfExpression) {
        std::stringstream exp;
        withExp->toString(exp);
        throw CException("The graph is not consistent for the set of scenario sequences formalized "
                         "by subexpression "
                         + CString(exp.str()));
    }

    auto *withoutExp = dynamic_cast<RegExScenarios *>((this->getWithoutOmega()).get());
    std::unique_ptr<RegExPersistentTokens> wo = withoutExp->checkConsistency();

    if (wo != nullptr) {
        if (wo->persistentTokensFinalOfExpression == w->persistentTokensOfExpression) {
            auto newRegExPersistentTokens = std::make_unique<RegExPersistentTokens>();
            newRegExPersistentTokens->persistentTokensFinalOfExpression =
                    w->persistentTokensFinalOfExpression;
            newRegExPersistentTokens->persistentTokensOfExpression =
                    wo->persistentTokensOfExpression;

            return newRegExPersistentTokens;
        }
        std::stringstream exp;
        this->toString(exp);
        throw CException("The graph is not consistent for the set of scenario sequences "
                         "formalized by subexpression "
                         + CString(exp.str()));
    }
    auto newRegExPersistentTokens = std::make_unique<RegExPersistentTokens>();
    newRegExPersistentTokens->persistentTokensFinalOfExpression =
            w->persistentTokensFinalOfExpression;
    newRegExPersistentTokens->persistentTokensOfExpression = w->persistentTokensOfExpression;

    return newRegExPersistentTokens;
}

std::unique_ptr<RegExPersistentTokens> OmegaChoiceS::checkConsistency() const {
    auto *leftExp = dynamic_cast<RegExScenarios *>((this->getLeft()).get());
    std::unique_ptr<RegExPersistentTokens> l = leftExp->checkConsistency();

    auto *rightExp = dynamic_cast<RegExScenarios *>((this->getRight()).get());
    std::unique_ptr<RegExPersistentTokens> r = rightExp->checkConsistency();

    if (l->persistentTokensOfExpression == r->persistentTokensOfExpression
        && l->persistentTokensFinalOfExpression == r->persistentTokensFinalOfExpression) {
        auto newRegExPersistentTokens = std::make_unique<RegExPersistentTokens>();
        newRegExPersistentTokens->persistentTokensFinalOfExpression =
                l->persistentTokensFinalOfExpression;
        newRegExPersistentTokens->persistentTokensOfExpression = l->persistentTokensOfExpression;

        return newRegExPersistentTokens;
    }
    std::stringstream exp;
    this->toString(exp);
    throw CException("The graph is not consistent for the set of scenario sequences formalized "
                     "by subexpression "
                     + CString(exp.str()));
}