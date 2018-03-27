#ifndef UPCOMINGMODEL_H
#define UPCOMINGMODEL_H

#include <QSettings>
#include <QSqlQueryModel>


class UpcomingModel : public QSqlQueryModel
{
    Q_OBJECT
public:
    enum class Mode {
        CONTACT_UPCOMING,
        TODAY,
        UPCOMING
    };

    enum Headers {
      H_ID,
      H_STATE,
      H_START_DATE,
      H_CONTACT_ID,
      H_CONTACT_NAME,
      H_CONTACT_STATUS,
      H_INTENT_ID,
      H_INTENT_ABSTRACT,
      H_PERSON_ID,
      H_PERSON_NAME,
      H_NAME,
      H_DUE_DATE,
      H_DESIRED_OUTCOME
    };

    constexpr static int NO_SELECTION = -1;

    UpcomingModel(QSettings& settings, QObject *parent, Mode mode);

    QVariant data(const QModelIndex &index, int role) const override;


public slots:
    void setContact(int contact = NO_SELECTION);
    void select();

private:
    QSqlQuery createQuery() const;

    const Mode mode_;
    QSettings& settings_;
    int contact_ = NO_SELECTION;
};

#endif // UPCOMINGMODEL_H
