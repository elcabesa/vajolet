#include "elo.h"
#include <cmath>

Elo::Elo(int wins, int losses, int draws)
{
	m_wins = wins;
	m_losses = losses;
	m_draws = draws;

	double n = wins + losses + draws;
	double w = wins / n;
	double l = losses / n;
	double d = draws / n;
	m_mu = w + d / 2.0;

	double devW = w * std::pow(1.0 - m_mu, 2.0);
	double devL = l * std::pow(0.0 - m_mu, 2.0);
	double devD = d * std::pow(0.5 - m_mu, 2.0);
	m_stdev = std::sqrt(devW + devL + devD) / std::sqrt(n);
}

double Elo::pointRatio() const
{
	double total = (m_wins + m_losses + m_draws) * 2;
	return ((m_wins * 2) + m_draws) / total;
}

double Elo::drawRatio() const
{
	double n = m_wins + m_losses + m_draws;
	return m_draws / n;
}

double Elo::diff() const
{
	return diff(m_mu);
}

double Elo::diff(double p)
{
	return -400.0 * std::log10(1.0 / p - 1.0);
}

double Elo::errorMargin() const
{
	double muMin = m_mu + phiInv(0.025) * m_stdev;
	double muMax = m_mu + phiInv(0.975) * m_stdev;
	return (diff(muMax) - diff(muMin)) / 2.0;
}

double Elo::erfInv(double x)
{
	const double pi = 3.1415926535897;

	double a = 8.0 * (pi - 3.0) / (3.0 * pi * (4.0 - pi));
	double y = std::log(1.0 - x * x);
	double z = 2.0 / (pi * a) + y / 2.0;

	double ret = std::sqrt(std::sqrt(z * z - y / a) - z);

	if (x < 0.0)
		return -ret;
	return ret;
}

double Elo::phiInv(double p)
{
	return std::sqrt(2.0) * erfInv(2.0 * p - 1.0);
}

double Elo::LOS() const
{
	return 100 * (0.5 + 0.5 * std::erf((m_wins - m_losses) / std::sqrt(2.0 * (m_wins + m_losses))));
}

