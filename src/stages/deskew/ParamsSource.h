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

#ifndef DESKEW_PARAMS_SOURCE_H_
#define DESKEW_PARAMS_SOURCE_H_

class QDomDocument;
class QDomElement;
class QString;

namespace deskew
{

class ParamsSource
{
public:
    ParamsSource();

    ParamsSource(double const& fov, bool const& photo);

    ParamsSource(QDomElement const& el);

    QDomElement toXml(QDomDocument& doc, QString const& name) const;

    double fov() const
    {
        return m_fov;
    }
    void setFov(double fov_value)
    {
        m_fov= fov_value;
    }

    bool photo() const
    {
        return m_photo;
    }
    void setPhoto(bool flg)
    {
        m_photo = flg;
    }

    bool operator==(ParamsSource const& other) const;

    bool operator!=(ParamsSource const& other) const;

private:
    double m_fov;
    bool m_photo;
};

} // namespace deskew

#endif /* DESKEW_SOURCE_PARAMS_H_ */
