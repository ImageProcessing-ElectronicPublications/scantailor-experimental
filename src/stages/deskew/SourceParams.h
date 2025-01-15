/*
    Scan Tailor - Interactive post-processing tool for scanned pages.
    Copyright (C) Joseph Artsimovich <joseph.artsimovich@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DESKEW_SOURCE_PARAMS_H_
#define DESKEW_SOURCE_PARAMS_H_

class QDomDocument;
class QDomElement;
class QString;

namespace deskew
{

class SourceParams
{
    // Member-wise copying is OK.
public:
    /**
     * Constructs RotationParams with MODE_AUTO and an invalid compensation angle.
     */
    SourceParams();

    SourceParams(double const& focus, bool const& photo);

    SourceParams(QDomElement const& el);

    QDomElement toXml(QDomDocument& doc, QString const& name) const;

    double focus() const
    {
        return m_focus;
    }
    void setFocus(double focus_value)
    {
        m_focus = focus_value;
    }

    bool photo() const
    {
        return m_photo;
    }
    void setPhoto(bool flg)
    {
        m_photo = flg;
    }

    bool operator==(SourceParams const& other) const;

    bool operator!=(SourceParams const& other) const;

private:
    double m_focus;
    bool m_photo;
};

} // namespace deskew

#endif /* DESKEW_SOURCE_PARAMS_H_ */
