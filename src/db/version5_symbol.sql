
ALTER TABLE optionChainStrikePrices
    ADD volatility real;

ALTER TABLE optionChainStrikePrices
    ADD callVolatility real;

ALTER TABLE optionChainStrikePrices
    ADD putVolatility real;

ALTER TABLE optionChainStrikePrices
    ADD itmProbability real;

ALTER TABLE optionChainStrikePrices
    ADD otmProbability real;
