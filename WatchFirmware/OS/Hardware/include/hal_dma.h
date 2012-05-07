#ifndef __HAL_DMA_H__
#define __HAL_DMA_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*DMACallback)(int channel);

void InitialiseDMA(void);
void SetDMAHandler(int channel, DMACallback callback);

#ifdef __cplusplus
}
#endif

#endif // __HAL_DMA_H__
